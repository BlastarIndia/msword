        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	rtfin_PCODE,rtfinn,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midRtfinn	equ 28		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<FStackRtfState>
externFP	<N_FcAppendRgchToFn>
externFP	<ErrorNoMemory>
externFP	<RtfInRare>
externFP	<N_FInsertRgch>
ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_FcAppendRgchToFn>
externFP	<S_FInsertRgch>
externFP	<ReportSzProc>
externFP	<LockHeap>
externFP	<UnlockHeap>
endif ;DEBUG


sBegin  data

; EXTERNALS

externW     mpdochdod
externW     vmerr
ifdef DEBUG
externW     cHpFreeze
externW     vhribl
endif ;DEBUG

sEnd    data

; CODE SEGMENT _EDIT

sBegin	rtfinn
	assumes cs,rtfinn
        assumes ds,dgroup
        assumes ss,dgroup


;-------------------------------------------------------------------------
;	RtfIn(hribl, pch, cch)
;-------------------------------------------------------------------------
;/*	 R T F	I N
;
;Inputs:
;	 hribl		 RTF IN Block
;	 pch		 RTF Text to input
;	 cch		 Length of input text
;
;Converts RTF text to internal form.
;*/
;
;RtfIn(hribl, pch, cch)
;struct RIBL **hribl;
;char *pch;
;int cch;
;{
;   char *pchLim;
;   char *pchMin;
;   char ch;
;   int ris;
;   int doc;
;   int  rds;
;   struct RIBL *pribl;

RIRisJmpTable:
	errnz	<risNorm - 0>
	dw	offset LrisNorm
	errnz	<risExit - 1>
	dw	offset RI58	; pop return address and goto LReturn
	errnz	<risB4ContSeq - 2>
	dw	offset LrisB4ContSeq
	errnz	<risContWord - 3>
	dw	offset LrisContWord
	errnz	<risNumScan - 4>
	dw	offset LrisNumScan
	errnz	<risScanPic - 5>
	dw	offset LrisScanPic
	errnz	<risB4Private1 - 6>
	dw	offset LrisB4Private1
	errnz	<risB4BinCode - 7>
	dw	offset LrisB4BinCode
	errnz	<risBinCode - 8>
	dw	offset LrisBinCode
	errnz	<risB4SpecXeTc - 9>
	dw	offset LrisB4SpecXeTc
	errnz	<risB4TableName - 10>
	dw	offset LrisB4TableName
	errnz	<risScanTableName - 11>
	dw	offset LrisScanTableName
	dw	offset LrisDefault

RIRdsCodes:
	db	rdsPic
	db	rdsPrivate1
	db	rdsXe
	db	rdsTc
	db	rdsStylesheet
	db	rdsFonttbl
	db	rdsColortbl
	db	rdsBkmkEnd
	db	rdsBkmkStart
	db	rdsFldType
	db	rdsGridtbl
RIRisNextCodes:
	db	risScanPic
	db	risB4Private1
	db	risB4SpecXeTc
	db	risB4SpecXeTc
	db	risB4TableName
	db	risB4TableName
	db	risB4TableName
	db	risB4TableName
	db	risB4TableName
	db	risB4TableName
	db	risB4TableName

RI01:
	jmp	RI59

cProc	N_RtfIn,<PUBLIC,FAR>,<si,di>
	ParmW	hribl
	ParmW	pch
	ParmW	cch

	LocalW	pchMin
	LocalW	pchLim
	LocalW	ris
	LocalW	doc
	LocalW	pchDataLim
ifdef DEBUG
	LocalV	rgchWarning,42
	LocalV	rgchFileName,11
endif ;DEBUG

cBegin

;#ifdef DEBUG
;   Assert (cbPAP >= cbCHP);
;   vhribl = hribl;
;#endif /* DEBUG */
ifdef DEBUG
	push	[hribl]
	pop	[vhribl]
endif ;DEBUG

;   pribl = *hribl;  /* local heap pointer usage only */
	mov	bx,[hribl]
	mov	bx,[bx]

;   if ((ris = pribl->ris) == risExit)
;	{
;	return;
;	}
	mov	al,[bx.risRibl]
	cmp	al,risExit
	je	RI01

;   FreezeHp();  /* for pribl */
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG
	mov	bptr ([ris]),al

;   doc = pribl->doc;
	mov	cx,[bx.docRibl]
	mov	[doc],cx

;   MeltHp();
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;   pchLim = pch + cch;
	mov	si,[pch]
	mov	cx,[cch]
	add	cx,si
	mov	[pchLim],cx

;   while (pch < pchLim && !vmerr.fDiskFail && !vmerr.fMemFail)
;	{ /* Loop for all characters in input stream */
RI02:
	cmp	si,[pchLim]
	jae	Ltemp003
	mov	ax,[vmerr.fDiskFailMerr]
	or	ax,[vmerr.fMemFailMerr]
	jne	Ltemp003

;	switch (ris)
;	    { /* Switch on state */
	mov	bl,bptr ([ris])
	xor	bh,bh
	cmp	bl,risScanTableName
	jbe	RI03
	mov	bl,risScanTableName + 1
RI03:
	shl	bx,1
	mov	ax,offset RI02
	push	ax	;default return to RI02
	mov	ax,cs:[bx.(offset RIRisJmpTable)]
	mov	bx,[hribl]
	mov	bx,[bx]
	jmp	ax
Ltemp003:
	push	si	; RI57 expects a word of junk on the stack
	jmp	RI57

;-------------------------------------------------------------------------
;	case risNorm: /* Normal characters; write them out */
;	    /* Save initial position; scan for escape or end;
;	       write out chars if any; return if at end;
;	       set up to parse pmc */
LrisNorm:

;	    rds = (**hribl).rds;
;	    pchMin = pch;
	mov	ax,[bx.rdsRibl]
	mov	[pchMin],si
ifdef DEBUG
	or	ah,ah
	je	RI04
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI04:
endif ;DEBUG
	push	cs
	pop	es
	mov	di,offset RIRdsCodes
	mov	cx,offset RIRisNextCodes - offset RIRdsCodes
	repne	scasb
	mov	al,risNorm
	jne	RI05
	mov	al,cs:[di + offset RIRisNextCodes - offset RIRdsCodes - 1]
RI05:
ifdef DEBUG
	or	al,al
	jns	RI055
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI055:
endif ;DEBUG
	cbw
	xchg	ax,dx	;save risNext in dx

;	    do
;		{
RI06:

;		ch = *pch++;
;		if ((**hribl).lcb > 0L)
;		    goto LRdsDispatch;
	mov	bx,[hribl]
	mov	bx,[bx]
	push	ds
	pop	es
	mov	di,si
	mov	cx,[pchLim]
	sub	cx,si
	mov	ah,bptr ([bx.chsRibl])
	lodsb
ifdef DEBUG
	cmp	[bx.HI_lcbRibl],0
	jge	RI065
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI065:
endif ;DEBUG
	dec	si
	push	ax
	mov	ax,[bx.LO_lcbRibl]
	or	ax,[bx.HI_lcbRibl]
	pop	ax
	jne	LRdsDispatch
	errnz	<risNorm - 0>
	or	dl,dl
	jne	RI12
RI07:
	;***Begin non rds switch loop
	; switch rds does not need to be performed in this loop because
	; the default (do nothing) case will always be taken.
;		switch (ch)
;		    {
;		case chRTFOpenBrk:
;		    if (!FStackRtfState(hribl))
;			{
;			Assert (vmerr.fMemFail);
;			goto LRetError;
;			}
;		    goto LBack1;
;		case chRTFCloseBrk:
;		    ris = risDoPop;
;		    goto LBack1;
;		case chBackslash:
;		    ris = risB4ContSeq;
;		    goto LBack1;
;		case chEop:
;		case chReturn:
;LBack1:
;		    --pch;
;		    goto LDoBreak;
	lodsb
	cmp	al,chEop
	je	RI09
	cmp	al,chReturn
	je	RI09
	cmp	al,'{'
	je	RI15
	cmp	al,'}'
	je	RI15
	cmp	al,'\'
	je	RI15

;		default:
;LRdsDispatch:
	;Assembler note: handle (**hribl).fInBody outside the inner loop.
;		    if (!(**hribl).fInBody && (**hribl).rds == rdsMain)
;			/* first RTF body text found */
;			(**hribl).fInBody = fTrue;
;
;		    if (ch >= 128 || ch < 32)
;			*(pch - 1) = (char)ChMapSpecChar(ch, (**hribl).chs);
;		    }
;		}
;	    while (pch < pchLim);
	;Assembler note: signed compare handles both ch < 32 and ch >= 128.
	cmp	al,32
	jl	RI11
RI08:
	stosb
RI09:
	loop	RI07
	jmp	short LDoBreak

RI11:
	;LN_ChMapSpecChar takes ch in al, chs in ah and returns
	;the mapped ch in al.  only al is altered.
	call	LN_ChMapSpecChar
	jmp	short RI08
	;***End non rds switch loop

RI12:
	;***Begin rds switch loop
	; switch rds does not need to be performed in this loop because
	; the default (do nothing) case will always be taken.
;		switch (ch)
;		    {
;		case chRTFOpenBrk:
;		    if (!FStackRtfState(hribl))
;			{
;			Assert (vmerr.fMemFail);
;			goto LRetError;
;			}
;		    goto LBack1;
;		case chRTFCloseBrk:
;		    ris = risDoPop;
;		    goto LBack1;
;		case chBackslash:
;		    ris = risB4ContSeq;
;		    goto LBack1;
;		case chEop:
;		case chReturn:
;LBack1:
;		    --pch;
;		    goto LDoBreak;
	lodsb
	cmp	al,chEop
	je	RI14
	cmp	al,chReturn
	je	RI14
	cmp	al,'{'
	je	RI15
	cmp	al,'}'
	je	RI15
	cmp	al,'\'
	je	RI15

;		default:
;LRdsDispatch:
;		    switch (rds)
;			{
;		    case rdsPic:
;			ris = risScanPic;
;			goto LBackUp;
;		    case rdsPrivate1:
;			ris = risB4Private1;
;			goto LBackUp;
;
;		    case rdsXe:
;		    case rdsTc:
;			  /* these must be escaped */
;			if (ch == chColon || ch == chDQuote)
;			    {
;			    ris = risB4SpecXeTc;
;			      /* *(pch -1) is the colon/quote.
;			      */
;			    --pch;
;			    goto LDoBreak;
;			    }
;			break;
;		    case rdsStylesheet:
;		    case rdsFonttbl:
;		    case rdsColortbl:
;
;		    case rdsBkmkEnd:
;		    case rdsBkmkStart:
;
;		    case rdsFldType:
;		    case rdsGridtbl:
;
;			 /* note: for these types we assume that there
;			    will be no imbedded backslashes or curly braces.
;			    If that changes, we must process these as we do the
;			    text info fields - dump to the doc and replace
;			    out
;			 */
;			ris = risB4TableName;
;LBackUp:
;			--pch;
;			goto LBreakNoPush;
;			}
	;Assembler note: handle (**hribl).fInBody outside the inner loop.
;		    if (!(**hribl).fInBody && (**hribl).rds == rdsMain)
;			/* first RTF body text found */
;			(**hribl).fInBody = fTrue;
;
;		    if (ch >= 128 || ch < 32)
;			*(pch - 1) = (char)ChMapSpecChar(ch, (**hribl).chs);
;		    }
;		}
;	    while (pch < pchLim);
	cmp	dl,risB4SpecXeTc
	jne	RI16
	cmp	al,':'
	je	RI16
	cmp	al,'"'
	je	RI16
	;Assembler note: signed compare handles both ch < 32 and ch >= 128.
	cmp	al,32
	jge	RI13
	;LN_ChMapSpecChar takes ch in al, chs in ah and returns
	;the mapped ch in al.  only al is altered.
	call	LN_ChMapSpecChar
RI13:
	stosb
RI14:
	loop	RI12
	db	0A8h	;turns next "dec si" into "test al,immediate"
RI15:
	;***End rds switch loop
	dec	si
	errnz	<risNorm - 0>
	xor	dl,dl
	db	0A8h	;turns next "dec si" into "test al,immediate"
RI16:
	dec	si

;LDoBreak:
LDoBreak:
LRdsDispatch:
;/* Note: it is safe to pass a chp stored on the heap to FInsertRgch. */
;	    if (pch != pchMin)
;		{
;		FInsertRgch(doc, CpMacDoc(doc) - ccpEop, pchMin, pch - pchMin,
;		    &(**hribl).chp, 0);
;		}
	cmp	al,'{'
	jne	RI18
	cCall	FStackRtfState,<[hribl]>
	mov	bx,[hribl]
	mov	bx,[bx]
	or	ax,ax
	jne	RI19
ifdef DEBUG
	cmp	[vmerr.fMemFailMerr],fFalse
	jne	RI17
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI17:
endif ;DEBUG
	jmp	LRetError
RI18:
	cmp	al,'}'
	je	RI187
	cmp	al,'\'
	je	RI183
	errnz	<risNorm - 0>
	or	dl,dl
	je	RI19
	cmp	dl,risB4SpecXeTc
	db	0B8h	;turns next "mov dl,immediate" into "mov ax,immediate"
RI183:
	mov	dl,risB4ContSeq
	db	0B8h	;turns next "mov dl,immediate" into "mov ax,immediate"
RI187:
	mov	dl,risDoPop
	mov	bptr ([ris]),dl
	jne	LBreakNoPush
RI19:
	cmp	di,[pchMin]
	je	RI22
	cmp	[bx.rdsRibl],rdsMain
	jne	RI20
	or	[bx.fInBodyRibl],maskFInBodyRibl
RI20:
	;LN_FInsertRgch takes pch in cx, cch in di, pribl in bx and performs
	;FInsertRgch(doc, CpMacDoc(doc) - ccpEop, pch, cch,
	;    pribl->chp, 0);
	;si is not altered.
	mov	cx,[pchMin]
	sub	di,cx
	call	LN_FInsertRgch
RI22:

;	    /* Skip over escape again - ignored if end of input */
;	    if (ris != risDoPop && ris != risB4SpecXeTc)
;		++pch;
	mov	al,bptr ([ris])
	cmp	al,risDoPop
	je	LBreakNoPush
	cmp	al,risB4SpecXeTc
	je	LBreakNoPush
	inc	si

;LBreakNoPush:
;	    break;
LBreakNoPush:
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;-------------------------------------------------------------------------
;	case risExit:
;	    goto LReturn;
	;Assembler note: the risExit case is handled at the jump table

;-------------------------------------------------------------------------
;	case risB4ContSeq:
;	    {
LrisB4ContSeq:

;	    pribl= *hribl;
;	    FreezeHp();
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG

;	    ch = *pch;
;	    if ((ch >= 'a' && ch <= 'z') || ch == ' ' ||
;		(ch >= 'A' && ch <= 'Z'))
;		{
;		/* true when letter or space */
;		ris = risContWord;
;		pribl->bchSeqLim = 0;
;		}
	mov	bptr ([ris]),risContWord
	mov	[bx.bchSeqLimRibl],0
	lodsb
	cmp	al,' '
	je	RI225
	call	LN_FAlphabetic
	jc	RI225

;	    else
;		{
;		char *pchSeqLim = pribl->rgch;	  /* HEAP pointer! */
;
;		*pchSeqLim++ = ch;
;		pch++;
;		*pchSeqLim = 0;
;		ris = risAfContSeq;
;		pribl->bchSeqLim = 1;
;		MeltHp();
;		break;
;		}
;	    MeltHp();
;	    }
	xor	ah,ah
	mov	wptr ([bx.rgchRibl]),ax
	mov	bptr ([ris]),risAfContSeq
	mov	[bx.bchSeqLimRibl],1
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
	db	0C3h	    ;near ret (ret generates far ret opcode here)
RI225:
	dec	si
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;-------------------------------------------------------------------------
;	case risContWord:
LrisContWord:

;	    {
;	    char *pchSeqLim;
;	    char *pchNumLim;
;	    pribl= *hribl;
;	    FreezeHp();
;	    pchSeqLim = pribl->rgch + pribl->bchSeqLim;
;	    pchNumLim = pribl->rgch + pribl->bchNumLim;
;	    do
;		{
	push	ds
	pop	es
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG
	mov	di,[bx.bchSeqLimRibl]
	add	bx,(rgchRibl)
	add	di,bx
	mov	cx,[pchLim]
	sub	cx,si
RI23:

;		int ch = *pch;
	lodsb

;		if (ch == '-' || (ch >= '0' && ch <= '9'))
;		    {
;		    /* fTrue when '-' or a digit. */
;		    *pchSeqLim++ = 0;
;		    pchNumLim = pchSeqLim;
;		    *pchNumLim++ = ch;
;		    pch++;
;		    ris = risNumScan;
;		    break;
;		    }
	cmp	al,'-'
	je	RI25
	cmp	al,'0'
	jb	RI24
	cmp	al,'9'
	jbe	RI25
RI24:

;		else if (ch == ' ')
;		    {
;		    pch++;
;		    goto LEndContWord;
;		    }
	cmp	al,' '
	je	RI26

;		else if (!((ch >= 'a' && ch <= 'z') ||
;		    (ch >= 'A' && ch <= 'Z')))
;		    {
;		    /* Note that CRLF will terminate the control word */
;		    /* true when not a letter and not a
;		       digit */
;LEndContWord:
;		    *pchSeqLim = 0;
;		    ris = risAfContSeq;
;		    break;
;		    }
;		*pchSeqLim++ = ch;
;		pch++;
;		}
	call	LN_FAlphabetic
	jnc	LEndContWord
	stosb
	loop	RI23

;	    while (pch < pchLim);
;	    pribl->bchSeqLim = pchSeqLim - pribl->rgch;
;	    pribl->bchNumLim = pchNumLim - pribl->rgch;
;	    MeltHp();
;	    break;
;	    }
	jmp	short RI28

RI25:
;		    /* fTrue when '-' or a digit. */
;		    *pchSeqLim++ = 0;
;		    pchNumLim = pchSeqLim;
;		    *pchNumLim++ = ch;
;		    pch++;
;		    ris = risNumScan;
;		    break;
;		    }
	mov	ah,al
	mov	al,0
	stosw
	mov	cx,di
	sub	cx,bx
	mov	wptr ([bx.bchNumLimRibl - (rgchRibl)]),cx
	mov	al,risNumScan
	jmp	short RI27

;LEndContWord:
;		    *pchSeqLim = 0;
;		    ris = risAfContSeq;
;		    break;
LEndContWord:
	dec	si
RI26:
	mov	al,0
	stosb
	mov	al,risAfContSeq
RI27:
	dec	di
	mov	bptr ([ris]),al
RI28:
	sub	di,bx
	mov	wptr ([bx.bchSeqLimRibl - (rgchRibl)]),di
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;-------------------------------------------------------------------------
;	case risNumScan:
;	    {
;	    char *pchNumLim;
;	    FreezeHp();
;	    pribl= *hribl;
;	    pchNumLim = pribl->rgch + pribl->bchNumLim;
;	    do
;		{
LrisNumScan:
	push	ds
	pop	es
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG
	mov	di,[bx.bchNumLimRibl]
	add	bx,([rgchRibl])
	add	di,bx
	mov	cx,[pchLim]
	sub	cx,si
RI29:

;		ch = *pch;
;		if (ch == ' ')
;		    {
;		    pch++;
;		    goto LEndNumScan;
;		    }
	lodsb
	cmp	al,' '
	je	RI32

;		if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') ||
;		    (ch >= 'A' && ch <= 'Z')))
;		    {
;		    /* true when not a letter and not a
;		       digit. Note a CRLF will end the num so be sure
;		       that generated control words are not CRLF interrupted
;		    */
;LEndNumScan:
;		    *pchNumLim = 0;
;		    ris = risAfContSeq;
;		    break;
;		    }
	cmp	al,'0'
	jb	RI30
	cmp	al,'9'
	jbe	RI31
RI30:
	call	LN_FAlphabetic
	jnc	LEndNumScan
RI31:

;		*pchNumLim++ = ch;
;		pch++;
;		}
;	    while (pch < pchLim);
	stosb
	loop	RI29

;	    pribl->bchNumLim = pchNumLim - pribl->rgch;
;	    MeltHp();
	jmp	short RI33

;	    break;
;	    }

LEndNumScan:
	dec	si
RI32:
	mov	al,0
	stosb
	dec	di
	mov	bptr ([ris]),risAfContSeq
RI33:
	sub	di,bx
	mov	wptr ([bx.bchNumLimRibl - (rgchRibl)]),di
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;-------------------------------------------------------------------------
;	case risScanPic:
;	    {
LrisScanPic:

;	    CHAR *pchData = (**hribl).rgch;
	;Assembler note: create pchData when needed, not here.

;	      /* lcb > 0 if binary data only. No spaces, cr/lf, etc allowed */
;	    if ((**hribl).lcb > 0L )
;		{
	mov	ax,[pchLim]
	sub	ax,si
ifdef DEBUG
	cmp	[bx.HI_lcbRibl],0
	jge	RI335
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI335:
endif ;DEBUG
	mov	cx,[bx.LO_lcbRibl]
	or	cx,[bx.HI_lcbRibl]
	je	RI37

;		if (!((**hribl).fDisregardPic))
;		    {
;		    do
;			{
	cwd
	mov	cx,ax
	sub	[bx.LO_lcbRibl],ax
	sbb	[bx.HI_lcbRibl],dx
	jge	RI34
	add	cx,[bx.LO_lcbRibl] ; lcb had been decremented. cx now original lcb
	xor	ax,ax
	mov	[bx.LO_lcbRibl],ax
	mov	[bx.HI_lcbRibl],ax
RI34:
	mov	dx,si
	add	si,cx
	test	[bx.fDisregardPicRibl],maskFDisregardPicRibl
	jne	Ltemp002

;			if (pchData >= (**hribl).rgch + cchMaxRibl)
;			    {
;			    int cbPicture = pchData - (**hribl).rgch;
;			    FcAppendRgchToFn(fnScratch,
;				 (**hribl).rgch, cbPicture);
;
;			    pchData = (**hribl).rgch;
;			    }
;			*pchData++ = *pch++;
;			(**hribl).lcb--;
;			}
;		    while (pch < pchLim && (**hribl).lcb > 0L);
	mov	ax,fnScratch
	push	ax
	push	dx
	push	cx
ifdef DEBUG
	cCall	S_FcAppendRgchToFn,<>
else ;!DEBUG
	cCall	N_FcAppendRgchToFn,<>
endif ;!DEBUG

;		    }
Ltemp002:
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;		else
;		    {
;		       /* there is no lmin */
;		    long lcch = CpMin((**hribl).lcb, (long)(pchLim - pch));
;		    (**hribl).lcb -= lcch;
;		    pch += (int)lcch;
;		    }
;		}
	;Assembler note: the else clause has already been done.

;	    else
;		{
	;Assembler note: the following code has been relocated from below.
RI35:
	mov	bptr ([ris]),risNorm
	dec	si
RI36:
	call	RI44
	mov	[bx.bPicRibl],dh    ;register bPic
	and	dl,maskFHiNibbleRibl + maskFDisregardPicRibl
	errnz	<(fHiNibbleRibl) - (fDisregardPicRibl)>
	mov	[bx.fHiNibbleRibl],dl
	db	0C3h	    ;near ret (ret generates far ret opcode here)

RI37:
;		do
;		    {
;		    ch = *pch;
	; We now have si = pch, ax = pchLim - pch
	xchg	ax,cx
	push	ds
	pop	es
	lea	di,[bx.rgchRibl]	;register pchData
	lea	dx,[di+cchMaxRibl]
	mov	[pchDataLim],dx
	errnz	<(fHiNibbleRibl) - (fDisregardPicRibl)>
	mov	dl,[bx.fHiNibbleRibl]
	mov	dh,[bx.bPicRibl]	;register bPic
RI38:
	lodsb				;register ch

;	     /*  handle picture switches. Normally this is done in the
;		main loop, but if processing a picture we have to jump into this
;		code and thus handle the switches ourselves.
;	     */
;		    if ( ch == chRTFOpenBrk || ch == chRTFCloseBrk ||
;		    ch == chBackslash)
;			{
;			ris = risNorm;
;			break;
;			}
	cmp	al,'{'
	je	RI35
	cmp	al,'}'
	je	RI35
	cmp	al,'\'
	je	RI35


;		    if (!((**hribl).fDisregardPic))
;			{
;			int w;
	test	dl,maskFDisregardPicRibl
	jne	LNextPicChar

;			if (ch >= '0' && ch <= '9')
;			    w = ch - '0';
;			else if (ch >= 'a' && ch <= 'f')
;			    w = 10 + ch - 'a';
;			else if (ch >= 'A' && ch <= 'F')
;			    w = 10 + ch - 'A';
	mov	ah,al
	sub	al,'0'
	cmp	al,'9' - '0'
	jbe	RI40
	sub	al,'a' - '0'
	cmp	al,'f' - 'a'
	jbe	RI39
	sub	al,'A' - 'a'
	cmp	al,'F' - 'A'
	jbe	RI39

;			else
;			    {
;			    /* ignore CR/LF, white space if non-binary picture */
;			    if (ch != chEop &&
;				ch != chReturn &&
;				ch != chTab &&
;				ch != chSpace)
;				/* bad data - pic discarded! */
;				{
	mov	al,ah
	cmp	al,chEop
	je	LNextPicChar
	cmp	al,chReturn
	je	LNextPicChar
	cmp	al,chTab
	je	LNextPicChar
	cmp	al,chSpace
	je	LNextPicChar

;				/* extra info at debug */
;				ReportSz("Warning - picture with bad data discarded");
ifdef DEBUG
	call	RI46
endif ;DEBUG

;				ErrorNoMemory(eidNoMemOperation);
	push	cx	;save pchLim - pch
	push	dx	;save flags
	sub	di,bx	;convert pchData to bchData
	sub	[pchDataLim],bx     ;convert pchDataLim to bchDataLim
	mov	ax,eidNoMemOperation
	cCall	ErrorNoMemory,<ax>
	push	ds
	pop	es
	pop	dx	;restore flags
	pop	cx	;restore pchLim - pch
	mov	bx,[hribl]
	mov	bx,[bx]
	add	di,bx	;restore pchData
	add	[pchDataLim],bx     ;restore pchDataLim

;				(**hribl).fDisregardPic = fTrue;
;				}
	or	dl,maskFDisregardPicRibl

;			    goto LNextPicChar;
;			    }
	jmp	short LNextPicChar
RI39:
	add	al,10
RI40:

;			(**hribl).bPic |= w << (4 * (**hribl).fHiNibble);
	errnz	<maskfHiNibbleRibl - 080h>
	xor	dl,maskfHiNibbleRibl
	js	RI41
	shl	al,1
	shl	al,1
	shl	al,1
	shl	al,1
	or	dh,al
	jmp	short RI43
RI41:
	or	al,dh

;			if (!(**hribl).fHiNibble)
;			    {
;			    if (pchData >= (**hribl).rgch + cchMaxRibl)
;				{
	cmp	di,[pchDataLim]
	jae	RI435

;				FcAppendRgchToFn(fnScratch,
;				   (**hribl).rgch, pchData - (**hribl).rgch);
;				pchData = (**hribl).rgch;
;				}
RI42:

;			    *pchData++ = (**hribl).bPic;
;			    (**hribl).bPic = 0;
;			    }
	stosb
	mov	dh,0

;			(**hribl).fHiNibble ^= 1;
;			}
RI43:
	;Assembler note: fHiNibble has already been xor'ed above

LNextPicChar:
;		    ++pch;
;		    }
;		while (pch < pchLim);
;		 } /* else */
	loop	RI38

;	    /* if macpict or pic args, pchData will still be ==
;	       **hribl.rgch, so FcAppend will not happen */
;
;	    if (pchData != (**hribl).rgch)
;		{
;		int cbPicture = pchData - (**hribl).rgch;
;		FcAppendRgchToFn(fnScratch,
;		      (**hribl).rgch, cbPicture);
;		}
;	    break;
;	    }
	jmp	RI36

RI435:
	call	RI44
	jmp	short RI42

	;Assembler note: this code has been relocated from above.
RI44:
	push	ax	;save ch
	push	cx	;save pchLim - pch
	push	dx	;save flags
	mov	ax,fnScratch
	mov	cx,di
	sub	di,bx	;convert pchData to bchData
	sub	[pchDataLim],bx     ;convert pchDataLim to bchDataLim
	add	bx,([rgchRibl])
	sub	cx,bx
	je	RI45
ifdef DEBUG
	cCall	S_FcAppendRgchToFn,<ax,bx,cx>
else ;!DEBUG
	cCall	N_FcAppendRgchToFn,<ax,bx,cx>
endif ;!DEBUG
RI45:
	push	ds
	pop	es
	pop	dx	;restore flags
	pop	cx	;restore pchLim - pch
	pop	ax	;restore ch
	mov	bx,[hribl]
	mov	bx,[bx]
	add	di,bx	;restore pchData
	add	[pchDataLim],bx     ;restore pchDataLim
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;				/* extra info at debug */
;				ReportSz("Warning - picture with bad data discarded");
ifdef DEBUG
RI46:
	push	cx
	push	si
	push	di
	push	ds
	push	es
	push	cs
	pop	ds
	push	ss
	pop	es
	mov	si,offset szWarning
	lea	di,[rgchWarning]
	mov	cx,cbSzWarning
	rep	movsb
	jmp	short RI47
szWarning:
	db	'Warning - picture with bad data discarded',0
cbSzWarning equ $ - szWarning
	errnz	<cbSzWarning - 42>
RI47:
	mov	si,offset szFileName
	lea	di,[rgchFileName]
	mov	cx,cbSzFileName
	rep	movsb
	jmp	short RI475
szFileName:
	db	'rtfinn.asm',0
cbSzFileName equ $ - szFileName
	errnz	<cbSzFileName - 11>
RI475:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx

	sub	di,bx	;convert pchData to bchData
	sub	[pchDataLim],bx     ;convert pchDataLim to bchDataLim
	push	ax	;save ch
	push	cx	;save pchLim - pch
	push	dx	;save flags
	lea	ax,[rgchWarning]
	push 	ax
	lea	ax,[rgchFileName]
	push 	ax
	mov	ax,1200   ; line number
	push 	ax
	cCall	ReportSzProc,<>
	push	ds
	pop	es
	pop	dx	;restore flags
	pop	cx	;restore pchLim - pch
	pop	ax	;restore ch
	mov	bx,[hribl]
	mov	bx,[bx]
	add	di,bx	;restore pchData
	add	[pchDataLim],bx     ;restore pchDataLim
	db	0C3h	    ;near ret (ret generates far ret opcode here)
endif ;DEBUG

;-------------------------------------------------------------------------
;	case risB4Private1:
LrisB4Private1:

;	    (**hribl).ibSea = 0;
;	    SetBytes((**hribl).rgbSea, 0, cbSEA);
	push	ds
	pop	es
	lea	di,[bx.ibSeaRibl]
	errnz	<(rgbSeaRibl) - (ibSeaRibl) - 2>
	errnz	<(cbSeaMin + 2) AND 1>
	mov	cx,(cbSeaMin + 2) SHR 1
	rep	stosw

;-------------------------------------------------------------------------
;	case risB4BinCode:
LrisB4BinCode:
;	    (**hribl).fHiNibble = fTrue;
;	    (**hribl).b = 0;
;	    ris = risBinCode;
	mov	[bx.bRibl],0
	or	[bx.fHiNibbleRibl],maskfHiNibbleRibl
	mov	bptr ([ris]),risBinCode

;-------------------------------------------------------------------------
;	case risBinCode:
;	    {
LrisBinCode:

;	    int w;
;	    ch = *pch++;
	lodsb

;	    if ((**hribl).rds == rdsPrivate1 && (ch == chRTFOpenBrk
;		     || ch == chRTFCloseBrk || ch == chBackslash))
;		{
;		ris = risNorm;
;		--pch;
;		break;
;		}
	cmp	[bx.rdsRibl],rdsPrivate1
	jne	RI49
	cmp	al,'{'
	je	RI48
	cmp	al,'}'
	je	RI48
	cmp	al,'\'
	jne	RI49
RI48:
	mov	bptr ([ris]),risNorm
	dec	si
	db	0C3h	    ;near ret (ret generates far ret opcode here)
RI49:

;	    w = 0;
;	    if (ch >= '0' && ch <= '9')
;		w = ch - '0';
;	    else if (ch >= 'a' && ch <= 'f')
;		w = 10 + ch - 'a';
;	    else if (ch >= 'A' && ch <= 'F')
;		w = 10 + ch - 'A';
	sub	al,'0'
	cmp	al,'9' - '0'
	jbe	RI51
	sub	al,'a' - '0'
	cmp	al,'f' - 'a'
	jbe	RI50
	sub	al,'A' - 'a'
	cmp	al,'F' - 'A'
	jbe	RI50
	mov	al,-10
RI50:
	add	al,10
RI51:

;	     /* broken up due to native compiler bug */
;	    (**hribl).b |= w << (4 * (**hribl).fHiNibble);
;	    (**hribl).fHiNibble ^= 1;
;	    if ((**hribl).fHiNibble)
;		{
	errnz	<maskfHiNibbleRibl - 080h>
	xor	[bx.fHiNibbleRibl],maskfHiNibbleRibl
	js	RI52
	shl	al,1
	shl	al,1
	shl	al,1
	shl	al,1
	or	[bx.bRibl],al
	db	0C3h	    ;near ret (ret generates far ret opcode here)
RI52:
	or	[bx.bRibl],al

;		if ((**hribl).rds == rdsPrivate1)
;		    {
	mov	al,[bx.bRibl]
	cmp	[bx.rdsRibl],rdsPrivate1
	jne	RI53

;		    if ((**hribl).ibSea < cbSEA)
;			(**hribl).rgbSea[(**hribl).ibSea++] = (**hribl).b;
;		    ris = risB4BinCode;
	mov	bptr ([ris]),risB4BinCode
	mov	di,[bx.ibSeaRibl]
	cmp	di,cbSeaMin
	jae	Ltemp001
	lea	di,[bx+di.rgbSeaRibl]
	mov	[di],al
	inc	[bx.ibSeaRibl]
Ltemp001:
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;		    }
;		else
;		    {
RI53:

;		    char chOut;
;		    chOut = (char)ChMapSpecChar((**hribl).b, (**hribl).chs);
	mov	ah,bptr ([bx.chsRibl])
	;LN_ChMapSpecChar takes ch in al, chs in ah and returns
	;the mapped ch in al.  only al is altered.
	call	LN_ChMapSpecChar

;		    FInsertRgch( doc, CpMacDocEdit(doc), &chOut, 1,
;			&(**hribl).chp, 0);
	;LN_FInsertRgch takes pch in cx, cch in di, pribl in bx and performs
	;FInsertRgch(doc, CpMacDoc(doc) - ccpEop, pch, cch,
	;    pribl->chp, 0);
	;si is not altered.
	mov	di,1
RI55:
	push	ax
	mov	cx,sp
	call	LN_FInsertRgch
	pop	bx	;remove chOut from the stack

;		    ris = risNorm;
	mov	bptr ([ris]),risNorm

;		    }
;		}
;	    }
;	    break;
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;-------------------------------------------------------------------------
;	case risB4SpecXeTc:
;	    {
LrisB4SpecXeTc:

;	    char rgchT[2];
;
;	    /* we have dumped up to the quote or colon to the doc;
;	       now insert a backslash and the character */
;	    Assert (*pch == chDQuote || *pch == chColon);
;	    *rgchT = chBackslash;
;	    *(rgchT + 1) = *pch;
	lodsb
ifdef DEBUG
	cmp	al,'"'
	je	RI56
	cmp	al,':'
	je	RI56
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI56:
endif ;DEBUG
	mov	ah,al
	mov	al,'\'

;	    FInsertRgch(doc, CpMacDoc(doc) - ccpEop, rgchT, 2,
;		    &(**hribl).chp, 0);
;	    ris = risNorm;
;	    ++pch;
;	    break;
;	    }
	mov	di,2
	jmp	short RI55

;-------------------------------------------------------------------------
;	case risB4TableName:
LrisB4TableName:

;	    (**hribl).bchData = 0;
	mov	[bx.bchDataRibl],0

;	    ris = risScanTableName;
	mov	bptr ([ris]),risScanTableName

;-------------------------------------------------------------------------
;	case risScanTableName:
LrisScanTableName:

;	    do
;		{
;		char ch;

	push	ds
	pop	es
	lea	di,[bx.rgchRibl]
	add	di,[bx.bchDataRibl]
	mov	cx,[pchLim]
	sub	cx,si
RI562:

;		ch = *pch;
;		if (ch == ';' || ch == chRTFCloseBrk ||
;		    ch == chRTFOpenBrk || ch == chBackslash)
;		    {
;		    if (ch == ';')
;		      ++pch;
;		    ris = risAddTableName;
;		    break;
;		    }
	lodsb
	cmp	al,';'
	je	RI568
	cmp	al,'}'
	je	RI566
	cmp	al,'{'
	je	RI566
	cmp	al,'\'
	je	RI566

;		*((**hribl).rgch + (**hribl).bchData++) = ch;
;		++pch;
;		} /* end do under risScanTableName */
;	    while (pch < pchLim);
	stosb
	loop	RI562
RI564:
	sub	di,bx
	sub	di,([rgchRibl])
	mov	[bx.bchDataRibl],di

;	    break;
	db	0C3h	    ;near ret (ret generates far ret opcode here)

RI566:
	dec	si
RI568:
	mov	bptr ([ris]),risAddTableName
	jmp	short RI564


;-------------------------------------------------------------------------
;	default:
;	    {
LrisDefault:

;	    pribl = *hribl;  /* local heap pointer usage only */
;	    pribl->ris = ris;
;	    pribl->bchSeqLim = pchSeqLim;
	mov	al,bptr ([ris])
	mov	[bx.risRibl],al

;	    RtfInRare(hribl, &pch, pchLim);
	mov	[pch],si
	lea	ax,[pch]
	mov	di,[hribl]
	cCall	RtfInRare,<di, ax, [pchLim]>
	mov	si,[pch]

;	    pribl = *hribl;  /* local heap pointer usage only */
;	    ris = pribl->ris;
	mov	bx,[di]
	mov	al,[bx.risRibl]
	mov	bptr ([ris]),al

;	    if (pribl->fCancel)
;		 goto LReturn;
	test	[bx.fCancelRibl],maskFCancelRibl
	jne	RI58

;	    }
;	    break;
;	    } /* end switch ris */
	db	0C3h	    ;near ret (ret generates far ret opcode here)

;	} /* end while pch < pchLim */
;LRetError:
RI57:
LRetError:

;   if (vmerr.fDiskFail || vmerr.fMemFail)
;	{
	mov	ax,[vmerr.fDiskFailMerr]
	or	ax,[vmerr.fMemFailMerr]
	je	LReturn

;	ErrorNoMemory(eidNoMemOperation);
	mov	ax,eidNoMemOperation
	cCall	ErrorNoMemory,<ax>

;	ris = risExit;
	mov	bptr ([ris]),risExit

;	(*hribl)->fCancel = fTrue;
	mov	bx,[hribl]
	mov	bx,[bx]
	or	[bx.fCancelRibl],maskFCancelRibl

;	}
RI58:
;LReturn:
LReturn:
	pop	si	;remove near return address from the stack

;   pribl = *hribl;  /* local heap pointer usage only */
;   FreezeHp();  /* for pribl */
	mov	si,[hribl]
	mov	si,[si]
ifdef	DEBUG
	call	LN_FreezeHp
endif ;DEBUG

;   pribl->ris = ris;
	mov	al,bptr ([ris])
	mov	[si.risRibl],al

;   MeltHp();
ifdef	DEBUG
	call	LN_MeltHp
endif ;DEBUG

;}
RI59:

cEnd

	;LN_FInsertRgch takes pch in cx, cch in di, pribl in bx and performs
	;FInsertRgch(doc, CpMacDoc(doc) - ccpEop, pch, cch,
	;    pribl->chp, 0);
	;si is not altered.
LN_FInsertRgch:
	push	si	;save caller's si
	mov	si,[doc]
	push	si
	;***Begin in-line CpMacDoc
;	 return(CpMacDoc(doc) - 2*ccpEop);
	shl	si,1
	mov	si,[si.mpdochdod]
ifdef DEBUG
	or	si,si
	jne	RI21
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midRtfinn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
RI21:
endif ;DEBUG
	mov	si,[si]
	mov	ax,-ccpEop-ccpEop-ccpEop
	cwd
	add	ax,[si.LO_cpMacDod]
	adc	dx,[si.HI_cpMacDod]
	;***End in-line CpMacDoc
	push	dx
	push	ax
	push	cx
	push	di
	add	bx,([chpRibl])
	push	bx
	xor	cx,cx
	push	cx
ifdef DEBUG
	cCall	S_FInsertRgch,<>
else ;!DEBUG
	cCall	N_FInsertRgch,<>
endif ;!DEBUG
	pop	si	;restore caller's si
	ret

; End of RtfIn


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


;csconst char mpchMACchANSI[128] =
;   {
mpchMACchANSI:
; /* 128   129	 130   131   132   133	 134   135  */
;   0xc4, 0xc5, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xe1,
db  0c4h, 0c5h, 0c7h, 0c9h, 0d1h, 0d6h, 0dch, 0e1h
; /* 136   137	 138   139   140   141	 142   143  */
;   0xe0, 0xe2, 0xe4, 0xe3, 0xe5, 0xe7, 0xe9, 0xe8,
db  0e0h, 0e2h, 0e4h, 0e3h, 0e5h, 0e7h, 0e9h, 0e8h
; /* 144   145	 146   147   148   149	 150   151  */
;   0xea, 0xe8, 0xed, 0xec, 0xee, 0xef, 0xf1, 0xf3,
db  0eah, 0e8h, 0edh, 0ech, 0eeh, 0efh, 0f1h, 0f3h
; /* 152   153	 154   155   156   157	 158   159  */
;   0xf2, 0xf4, 0xf6, 0xf5, 0xfa, 0xf9, 0xfb, 0xfc,
db  0f2h, 0f4h, 0f6h, 0f5h, 0fah, 0f9h, 0fbh, 0fch
; /* 160   161	 162   163   164   165	 166   167  */
;   0xa0, 0xb0, 0xa2, 0xa3, 0xa7, 0x6f, 0xb6, 0xdf,
db  0a0h, 0b0h, 0a2h, 0a3h, 0a7h, 06fh, 0b6h, 0dfh
; /* 168   169	 170   171   172   173	 174   175  */
;   0xae, 0xa9, 0xaa, 0xb4, 0xa8, 0xad, 0xc6, 0xd8,
db  0aeh, 0a9h, 0aah, 0b4h, 0a8h, 0adh, 0c6h, 0d8h
; /* 176   177	 178   179   180   181	 182   183  */
;   0xb0, 0xb1, 0xb2, 0xb3, 0xa5, 0xb5, 0xb6, 0xb7,
db  0b0h, 0b1h, 0b2h, 0b3h, 0a5h, 0b5h, 0b6h, 0b7h
; /* 184   185	 186   187   188   189	 190   191  */
;   0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xc6, 0xd8,
db  0b8h, 0b9h, 0bah, 0bbh, 0bch, 0bdh, 0c6h, 0d8h
; /* 192   193	 194   195   196   197	 198   199  */
;   0xbf, 0xa1, 0xac, 0xc3, 0xc4, 0xc5, 0xc6, 0xab,
db  0bfh, 0a1h, 0ach, 0c3h, 0c4h, 0c5h, 0c6h, 0abh
; /* 200   201	 202   203   204   205	 206   207  */
;   0xbb, 0xc9, 0xa0, 0xc0, 0xc3, 0xd5, 0xce, 0xcf,
db  0bbh, 0c9h, 0a0h, 0c0h, 0c3h, 0d5h, 0ceh, 0cfh
; /* 208   209	 210   211   212   213	 214   215  */
;    /* note special printing char mapping in this line */
;   0x97, 0x96, 0x93, 0x94, 0x91, 0x92, 0xd6, 0xd7,
db  097h, 096h, 093h, 094h, 091h, 092h, 0d6h, 0d7h
; /* 216   217	 218   219   220   221	 222   223  */
;   0xff, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
db  0ffh, 0d9h, 0dah, 0dbh, 0dch, 0ddh, 0deh, 0dfh
; /* 224   225	 226   227   228   229	 230   231  */
;   0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
db  0e0h, 0e1h, 0e2h, 0e3h, 0e4h, 0e5h, 0e6h, 0e7h
; /* 232   233	 234   235   236   237	 238   239  */
;   0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
db  0e8h, 0e9h, 0eah, 0ebh, 0ech, 0edh, 0eeh, 0efh
; /* 240   241	 242   243   244   245	 246   247  */
;   0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
db  0f0h, 0f1h, 0f2h, 0f3h, 0f4h, 0f5h, 0f6h, 0f7h
; /* 248   249	 250   251   252   253	 254   255  */
;   0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
db  0f8h, 0f9h, 0fah, 0fbh, 0fch, 0fdh, 0feh, 0ffh
;   };

;csconst char mpchIBMchANSI[128] =
;   {
mpchIBMchANSI:
; /* 128   129	 130   131   132   133	 134   135  */
;   0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7,
db  0c7h, 0fch, 0e9h, 0e2h, 0e4h, 0e0h, 0e5h, 0e7h
; /* 136   137	 138   139   140   141	 142   143  */
;   0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,
db  0eah, 0ebh, 0e8h, 0efh, 0eeh, 0ech, 0c4h, 0c5h
; /* 144   145	 146   147   148   149	 150   151  */
;   0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9,
db  0c9h, 0e6h, 0c6h, 0f4h, 0f6h, 0f2h, 0fbh, 0f9h
; /* 152   153	 154   155   156   157	 158   159  */
;   0xff, 0xd6, 0xdc, 0xa2, 0xa3, 0xa5, 0x70, 0x66,
db  0ffh, 0d6h, 0dch, 0a2h, 0a3h, 0a5h, 070h, 066h
; /* 160   161	 162   163   164   165	 166   167  */
;   0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba,
db  0e1h, 0edh, 0f3h, 0fah, 0f1h, 0d1h, 0aah, 0bah
; /* 168   169	 170   171   172   173	 174   175  */
;   0xbf, 0x5f, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,
db  0bfh, 05fh, 0ach, 0bdh, 0bch, 0a1h, 0abh, 0bbh
; /* 176   177	 178   179   180   181	 182   183  */
;   0x5f, 0x5f, 0x5f, 0xa6, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 0a6h, 05fh, 05fh, 05fh, 05fh
; /* 184   185	 186   187   188   189	 190   191  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh
; /* 192   193	 194   195   196   197	 198   199  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh
; /* 200   201	 202   203   204   205	 206   207  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh
; /* 208   209	 210   211   212   213	 214   215  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh
; /* 216   217	 218   219   220   221	 222   223  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xa6, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 05fh, 05fh, 0a6h, 05fh, 05fh
; /* 224   225	 226   227   228   229	 230   231  */
;   0x5f, 0xdf, 0x5f, 0xb6, 0x5f, 0x5f, 0xb5, 0x5f,
db  05fh, 0dfh, 05fh, 0b6h, 05fh, 05fh, 0b5h, 05fh
; /* 232   233	 234   235   236   237	 238   239  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh
; /* 240   241	 242   243   244   245	 246   247  */
;   0x5f, 0xb1, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f,
db  05fh, 0b1h, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh
; /* 248   249	 250   251   252   253	 254   255  */
;   0xb0, 0x95, 0xb7, 0x5f, 0x6e, 0xb2, 0xa8, 0x5f
db  0b0h, 095h, 0b7h, 05fh, 06eh, 0b2h, 0a8h, 05fh
;   };


;csconst char mpchPCAchANSI[128] =
;   {
mpchPCAchANSI:
; /* 128   129	 130   131   132   133	 134   135  */
;   0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7,
db  0c7h, 0fch, 0e9h, 0e2h, 0e4h, 0e0h, 0e5h, 0e7h
; /* 136   137	 138   139   140   141	 142   143  */
;   0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5,
db  0eah, 0ebh, 0e8h, 0efh, 0eeh, 0ech, 0c4h, 0c5h
; /* 144   145	 146   147   148   149	 150   151  */
;   0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9,
db  0c9h, 0e6h, 0c6h, 0f4h, 0f6h, 0f2h, 0fbh, 0f9h
; /* 152   153	 154   155   156   157	 158   159  */
;   0xff, 0xd6, 0xdc, 0xf8, 0xa3, 0xd8, 0x5f, 0x66,
db  0ffh, 0d6h, 0dch, 0f8h, 0a3h, 0d8h, 05fh, 066h
; /* 160   161	 162   163   164   165	 166   167  */
;   0xe1, 0xed, 0xf3, 0xfa, 0xf1, 0xd1, 0xaa, 0xba,
db  0e1h, 0edh, 0f3h, 0fah, 0f1h, 0d1h, 0aah, 0bah
; /* 168   169	 170   171   172   173	 174   175  */
;   0xbf, 0xae, 0xac, 0xbd, 0xbc, 0xa1, 0xab, 0xbb,
db  0bfh, 0aeh, 0ach, 0bdh, 0bch, 0a1h, 0abh, 0bbh
; /* 176   177	 178   179   180   181	 182   183  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xc1, 0xc2, 0xc0,
db  05fh, 05fh, 05fh, 05fh, 05fh, 0c1h, 0c2h, 0c0h
; /* 184   185	 186   187   188   189	 190   191  */
;   0xa9, 0x5f, 0x5f, 0x5f, 0x5f, 0xa2, 0xa5, 0x5f,
db  0a9h, 05fh, 05fh, 05fh, 05fh, 0a2h, 0a5h, 05fh
; /* 192   193	 194   195   196   197	 198   199  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xe3, 0xc3,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 0e3h, 0c3h
; /* 200   201	 202   203   204   205	 206   207  */
;   0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0xa4,
db  05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 05fh, 0a4h
; /* 208   209	 210   211   212   213	 214   215  */
;   0xf0, 0xd0, 0xca, 0xcb, 0xc8, 0x5f, 0xcd, 0xce,
db  0f0h, 0d0h, 0cah, 0cbh, 0c8h, 05fh, 0cdh, 0ceh
; /* 216   217	 218   219   220   221	 222   223  */
;   0xcf, 0x5f, 0x5f, 0x5f, 0x5f, 0xab, 0xcc, 0x5f,
db  0cfh, 05fh, 05fh, 05fh, 05fh, 0abh, 0cch, 05fh
; /* 224   225	 226   227   228   229	 230   231  */
;   0xd3, 0xdf, 0xd4, 0xd2, 0xf5, 0xd5, 0xb5, 0xde,
db  0d3h, 0dfh, 0d4h, 0d2h, 0f5h, 0d5h, 0b5h, 0deh
; /* 232   233	 234   235   236   237	 238   239  */
;   0xfe, 0xda, 0xdb, 0xd9, 0xfd, 0xdd, 0xaf, 0xb4,
db  0feh, 0dah, 0dbh, 0d9h, 0fdh, 0ddh, 0afh, 0b4h
; /* 240   241	 242   243   244   245	 246   247  */
;   0xad, 0xb1, 0x3d, 0xbe, 0xb6, 0xa7, 0x5f, 0xb8,
db  0adh, 0b1h, 03dh, 0beh, 0b6h, 0a7h, 05fh, 0b8h
; /* 248   249	 250   251   252   253	 254   255  */
;   0xb0, 0xa8, 0x95, 0xb9, 0xb3, 0xb2, 0xa8, 0x5f
db  0b0h, 0a8h, 095h, 0b9h, 0b3h, 0b2h, 0a8h, 05fh
;   };

;-------------------------------------------------------------------------
;	ChMapSpecChar(ch, chs)
;-------------------------------------------------------------------------
;char ChMapSpecChar(ch, chs)
;char ch;
;int chs;
;{
ifdef	DEBUG
; %%Function:N_ChMapSpecChar %%Owner:BRADV
cProc	N_ChMapSpecChar,<PUBLIC,FAR>,<>
	ParmW	chArg
	ParmW	chsArg

cBegin
	mov	ax,[chArg]
	mov	ah,bptr [chsArg]
	call	LN_ChMapSpecChar
	xor	ah,ah
cEnd
endif ;DEBUG

	;LN_ChMapSpecChar takes ch in al, chs in ah and returns
	;the mapped ch in al.  only al is altered.
LN_ChMapSpecChar:

;   switch (chs)
;	{
	push	bx
	;Assembler note: signed compare takes care of both ch < 32
	;and ch >= 128.
	cmp	al,32
	jge	CMSC09

;   case chsAnsi:
;	if (ch < 32)
;	    ch = '_'; /* underscore for unrecognized low chars */
;	break;
	cmp	ah,chsAnsi
	jne	CMSC02
	cmp	al,32
	jae	CMSC09
CMSC01:
	mov	al,'_'
	jmp	short CMSC09
CMSC02:

;   case chsMac:
;	if (ch >= 128)
;	    ch = mpchMACchANSI[ch-128];
;	else if (ch < 32)
;       ch = '_'; /* underscore for unrecognized low chars */
;	break;
	errnz	<chsMac - 0>
	or	ah,ah
	jne	CMSC04
	mov	bx,offset [mpchMACchANSI - 128]
	or	al,al
	jns	CMSC01
CMSC03:
	xlat	cs:[bx]
	jmp	short CMSC09
CMSC04:

;   case chsPC:
;	if (ch >= 128)
;	    ch = mpchIBMchANSI[ch-128];
	cmp	ah,chsPC
	jne	CMSC08
	mov	bx,offset [mpchIBMchANSI - 128]
CMSC05:
	or	al,al
	js	CMSC03

;	else if (ch < 32)
;	    {
;LSpecLow:
;	    if (ch == 20)
;		ch = 182;
	cmp	al,20
	je	CMSC07

;	    else if (ch == 21)
;		ch = 167;
	cmp	al,21
	je	CMSC06

;	    else if (ch == 15)
;		ch = 150;
;	    else
;		ch = '_'; /* underscore for unrecognized low chars */
;	    }
;	break;
	cmp	al,15
	jne	CMSC01
	mov	al,150
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
CMSC06:
	mov	al,167
	db	03Dh	;turns next "mov al,immediate" into "cmp ax,immediate"
CMSC07:
	mov	al,182
	jmp	short CMSC09

CMSC08:
;   case chsPCA:
;	if (ch >= 128)
;	    ch = mpchPCAchANSI[ch-128];
;	else if (ch < 32)
;	    goto LSpecLow;
;	break;
;	}
	mov	bx,offset [mpchPCAchANSI - 128]
	cmp	ah,chsPCA
	je	CMSC05

;   return (ch);
CMSC09:

;}
	pop	bx
	ret


LN_FAlphabetic:
	push	ax
	sub	al,'a'
	cmp	al,'z' - 'a' + 1
	jc	FA01
	sub	al,'A' - 'a'
	cmp	al,'Z' - 'A' + 1
FA01:
	pop	ax
	ret

sEnd	rtfinn
        end
