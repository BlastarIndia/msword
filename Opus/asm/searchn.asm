;  SEARCHN.ASM	--  hand native search routines
;

	.xlist
	memS	= 1
	?WIN	= 1
	?PLM	= 1
	?NODATA = 1
	?TF	= 1
	include w2.inc
	include cmacros.inc
	include consts.inc
	include structs.inc
	.list

createSeg	search_PCODE,search,byte,public,CODE
createSeg	_DATA,data,word,public,DATA,DGROUP
defGrp		DGROUP,DATA

; DEBUGGING DECLARATIONS

ifdef DEBUG
midSearchn	  equ 8 	  ; module ID, for native asserts
endif ;DEBUG

; EXPORTED LABELS


; EXTERNAL FUNCTIONS

externFP	<FQueryAbortCheckProc>
externFP	<N_FetchCpPccpVisible>
externFP	<N_FetchCp>
externFP	<ProgressReportPercent>
externFP	<ReloadSb>
externFP	<ChUpper>
externFP	<AssureLegalSel>
externFP	<CacheTc>
externFP	<N_FInTableDocCp>

ifdef DEBUG
externFP	<AssertProcForNative>
externFP	<S_FetchCpPccpVisible>
externFP	<S_FetchCp>
externFP	<S_FInTableDocCp>
endif ;DEBUG

; EXTERNAL DATA


sBegin	data

externW vchpFetch	;extern struct CHP   vchpFetch;
externW vpapFetch	;extern struct PAP   vpapFetch;
externW vfbSearch	;extern struct FB    vfbSearch;
externW vfSearchCase	;extern BOOL	     vfSearchCase;
externD vcpLimWrap	;extern CP	     vcpLimWrap;
externD vcpFetch	;extern CP	     vcpFetch;
externW vccpFetch	;extern int	     vcpFetch;
externD vhpchFetch	;extern char HUGE *  vhpchFetch;
externW selCur		;extern SEL	     selCur;
externW caPara		;extern CA	     caPara;
externD vcpMatchLim	;extern CP	     vcpMatchLim;
externW mpsbps		;extern SB	     mpsbps[];
externW vtcc		;extern struct TCC   vtcc;
externW vfReplace	;BOOL	 vfReplace;
externW vfConfirm	;extern BOOL vfConfirm;
externW vpisPrompt	;extern int	     vpisPrompt;
externW vprri		;extern struct RRI   vprri;

sEnd	data

; CODE SEGMENT _SEARCH

sBegin	search
	assumes cs,search
	assumes ds,dgroup
	assumes ss,dgroup

PUBLIC LN_FMatchChp
PUBLIC LN_FMatchPap
PUBLIC LN_FetchCpPccpVisibleBackward
ifdef DEBUG
PUBLIC LN_CheckRgcpFetch
endif ;/* DEBUG */
PUBLIC LN_FMatchWhiteSpace


;-------------------------------------------------------------------------
;	FMatchChp()
;-------------------------------------------------------------------------
;NATIVE BOOL FMatchChp()
;{
;    int *pchpFetch = &vchpFetch;   /* !! Assumes vchpFetch has been set up */
;    int *pchpSearch = &vfbSearch.chp;
;    int *pchpGray = &vfbSearch.chpGray;
;    int cw = cwCHP;
;    int wT = 0;
;
;    while (cw--)
;	 wT |= ((*pchpFetch++ ^ *pchpSearch++) & (~*pchpGray++));
;
;    return (!wT);
;}

; %%Function:FMatchCp %%Owner:BRADV
cProc	FMatchChp,<PUBLIC,FAR>,<>

cBegin
	cCall	LN_FMatchChp,<>
cEnd


;cProc	 LN_FMatchChp,<PUBLIC,NEAR>,<si,di>
LN_FMatchChp LABEL NEAR

;cBegin

	push	si
	push	di
	mov	si,dataoffset [vchpFetch]
	mov	di,dataoffset [vfbSearch.chpFb]
	mov	bx,dataoffset [vfbSearch.chpGrayFb]
	mov	cx,cwCHP		; cw = cwCHP
	jmp	short LN_FMatch

;-------------------------------------------------------------------------
;	FMatchPap()
;-------------------------------------------------------------------------
;NATIVE BOOL FMatchPap()
;{
;    int *ppapFetch = &vpapFetch;   /* !! Assumes vpapFetch has been set up */
;    int *ppapSearch = &vfbSearch.pap;
;    int *ppapGray = &vfbSearch.papGray;
;    int cw = cwPAPS;
;    int wT = 0;
;
;    while (cw--)
;	 wT |= ((*ppapFetch++ ^ *ppapSearch++) & (~*ppapGray++));
;
;    return (!wT);
;}

; %%Function:FMatchPap %%Owner:BRADV
cProc	FMatchPap,<PUBLIC,FAR>,<>

cBegin
	cCall	LN_FMatchPap,<>
cEnd


;cProc	 LN_FMatchPap,<PUBLIC,NEAR>,<>
LN_FMatchPap LABEL NEAR

;cBegin

	push	si
	push	di
	mov	si,dataoffset [vpapFetch]
	mov	di,dataoffset [vfbSearch.papFb]
	mov	bx,dataoffset [vfbSearch.papGrayFb]
	mov	cx,cwPAPS		; cw = cwPAPS

LN_FMatch:
;   The following is an implementation of the following C code:
;
;   {
;   cw = cwStruct
;   wT = 0;
;
;   while (cw--)
;	wT |= ((*pwFetch++ ^ *pwSearch++) & (~*pwGray++));
;
;   return (!wT);
;   }
;
;   Note:  By DeMorgan's Equivalence,  (a & ~b) == ~(~a | b)
;	   so the line "wT |= ..." can be rewritten as:
;
;	   wT |= ~(~(*pwFetch++ ^ *pwSearch++) | *pwGray++);
;
;	   Why bother?	So we can code the loop using only one
;	   temporary value and thus enable it to be done without
;	   accessing memory inside the loop.  A clear win.

	xor	dx,dx			; wT = 0

LN_FMatchLoop:

	lodsw				; get *pwFetch; pwFetch++
	xor	ax,[di] 		; xor it with *pwSearch
	inc	di			; pwSearch++
	inc	di
	not	ax			; ~ the result
	or	ax,[bx] 		; or with *pwGray
	inc	bx			; pwGray++
	inc	bx
	not	ax			; ~ the result
	or	dx,ax			; wT |= ax

	loop	LN_FMatchLoop		; loop if (cw-- != 0)

	;NOTE: we know that cx == 0 at this point
	sub	dx,1			; set up return value
	adc	cx,0
	xchg	ax,cx

	pop	di
	pop	si
        ret

;cEnd


;-------------------------------------------------------------------------
;	LN_SetEsFetch
;-------------------------------------------------------------------------
	;LN_SetEsFetch takes sets es according to the sb of vhpchFetch.
	;Only es and bx are altered.
; %%Function:LN_SetEsFetch %%Owner:BRADV
cProc	LN_SetEsFetch,<NEAR,ATOMIC>,<>

cBegin	nogen
	mov	bx,whi ([vhpchFetch])
	push	ax
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	LN_RS01
;	ReloadSb trashes ax, cx, and dx
	push	cx
	push	dx
	cCall	ReloadSb,<>
	pop	dx
	pop	cx
LN_RS01:
	pop	ax
cEnd	nogen
	ret


maskFSearchCaseLocal	    equ 001h
maskFMatchedWhiteLocal	    equ 002h
maskFMatchFoundNonTextLocal equ 008h
maskFNotPlainLocal	    equ 080h

;-------------------------------------------------------------------------
;	CpSearchSz(pbmib, cpFirst, cpLim, pcpNextRpt, hppr)
;-------------------------------------------------------------------------
;CP CpSearchSz (pbmib, cpFirst, cpLim, pcpNextRpt, hppr)
;struct BMIB *pbmib;
;CP	 cpFirst;
;CP	 cpLim;
;CP	 *pcpNextRpt;
;struct  PPR **hppr;
;{
;/*  *** CpSearchSz ***
;
;    Finds the next occurrence of *pwPatFirst in the current document
;    starting at cpFirst. Ignores case of letters if vfSearchCase is FALSE.
;
;    This search procedure uses the Boyer/Moore algorithm: *pwPatFirst is
;    compared against the document from end to front - when a mismatch
;    occurs, the search jumps ahead mpchdcp[chDoc] CP's, where mpchdcp is
;    an array storing the minimum # of characters between the last char
;    and any occurrence of chDoc in *pwPatFirst. If there are no such
;    occurrences, we can jump the length of *pwPatFirst.
;
;    Returns the first cp of the text match in the document; cpNil if
;    not found.
;  */
;
;    int    *pwPatFirst = pbmib->rgwSearch;  /* local for speed */
;    char   *mpchdcp = pbmib->mpchdcp;	     /* local for speed */
;    int    chDoc;
;    char   HUGE *hpchDoc;
;    BOOL   fMatchedWhite;
;    int    cchMatched;
;    CP     cpRun, cpMatchStart;
;    int    ccpFetch;  /* local instead of vccpFetch */
;    int    ichDoc, cchT;
;    int    wPat, *pwPat;
;    int    ccpRun, ccpRemain;
;    int    dwrgwPat;
;    int    ichPat;
;    int    *pwPatLast;
;    BOOL   fMatchFoundNonText = fFalse;
;    CP     cpFirstInit = cpFirst;
;    struct CA caT;
;    BOOL   fTableFirst, fTableLim;
;    CP     cpLimOrig = cpLim;
;	BOOL   fTableCur;
; %%Function:N_CpSearchSz %%Owner:BRADV
cProc	N_CpSearchSz,<PUBLIC,FAR>,<si,di>
	ParmW	<pbmib>
	ParmD	<cpFirst>
	ParmD	<cpLim>
	ParmW	<pcpNextRpt>
	ParmW	<hppr>

	LocalW	pwPatFirst
	LocalW	mpchdcp
	LocalW	cchMatched
	LocalD	cpRun
	LocalW	ccpFetch
	LocalW	ccpRun
	LocalW	dbrgwPat
	LocalW	pwPatLast
	LocalD	cpFirstInit
	LocalW	cwSearch
	LocalW	wFlags
	LocalD	cpLimOrig
	LocalV	caT,cbCaMin
	LocalW	fTableCur

cBegin
	mov	bx,[pbmib]

;    Assert (!pbmib->fNotPlain || !pbmib->fChTableInPattern);
ifdef DEBUG
	cmp	[bx.fNotPlainBmib],fFalse
	je	LF005
	cmp	[bx.fChTableInPatternBmib],fFalse
	je	LF005
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midSearchn
	mov	bx,1032
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF005:
endif ;/* DEBUG */

	errnz	<(rgwSearchBmib) - 0>
	mov	[pwPatFirst],bx
	lea	ax,[bx.mpchdcpBmib]
	mov	[mpchdcp],ax

;    dwrgwPat = ((int)pbmib->rgwOppCase - (int)pwPatFirst) / sizeof(int);
	mov	[dbrgwPat],(rgwOppCaseBmib)-(rgwSearchBmib)

;    fMatchedWhite = fFalse;
;    fMatchFoundNonText = fFalse;
	;simultaneously put vfSearchCase in register
	;and clear fMatchedWhite, fMatchFoundNonText
	mov	ax,[vfSearchCase]
ifdef DEBUG
;	Make sure vfSearchCase exists only in the low bit.
	test	ax,NOT maskFSearchCaseLocal
	je	LF01
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midSearchn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF01:
endif ;/* DEBUG */
ifdef DEBUG
;	Make sure pbmib->fNotPlain exists only in the low bit.
	test	[bx.fNotPlainBmib],0FFFEh
	je	LF015
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midSearchn
	mov	bx,1030
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF015:
endif ;/* DEBUG */
	errnz	<maskFNotPlainLocal - 080h>
	mov	cl,bptr ([bx.fNotPlainBmib])
	ror	cl,1
	or	al,cl
	mov	[wFlags],ax
	mov	ax,[bx.cwSearchBmib]
	mov	[cwSearch],ax

;    pwPatLast = &pwPatFirst[pbmib->cwSearch - 1];
	errnz	<(rgwSearchBmib) - 0>
	dec	ax
	add	bx,ax
	add	bx,ax
	mov	[pwPatLast],bx

;    vcpLimWrap = CpMin(cpFirst + pbmib->cwSearch - 1, cpLim);
	mov	si,[OFF_cpLim]
	mov	di,[SEG_cpLim]
	;frame initialization of cpLimOrig done here
	mov	[OFF_cpLimOrig],si
	mov	[SEG_cpLimOrig],di
	cwd
	add	ax,[OFF_cpFirst]
	adc	dx,[SEG_cpFirst]
	mov	bx,ax
	mov	cx,dx
	sub	bx,si
	sbb	cx,di
	jl	LF02
	mov	ax,si
	mov	dx,di
LF02:
	mov	wlo [vcpLimWrap],ax
	mov	whi [vcpLimWrap],dx

;    cpRun = cpFirst;
	mov	ax,[OFF_cpFirst]
	mov	dx,[SEG_cpFirst]
	;frame initialization of cpFirstInit also done here
	mov	[OFF_cpFirstInit],ax
	mov	[SEG_cpFirstInit],dx

;    goto LFResetSearch;
	;	LFResetSearch expects cpRun in dx:ax
	jmp	LFResetSearch

;	 if (wPat == wMatchAny && chDoc != chTable)
;	     goto LFCharMatch;
LF025:
	cmp	dl,chTable
	je	LF097
	jmp	short Ltemp004

LF03:
;	 if (wPat == wMatchWhite && FMatchWhiteSpace(chDoc))
;	     {
;	     cchMatched++;
;	     fMatchedWhite = fTrue;
;	     goto LFBackUp;
;	     }
	;LN_FMatchWhiteSpace expects chDoc in dl, and returns equal if true.
	;No registers are changed.
	cCall	LN_FMatchWhiteSpace,<>
	jne	LF10
	or	al,maskfMatchedWhiteLocal
LF04:
	inc	cchMatched;
	jmp	LFBackUp

;	 if (wPat == chSpace && chDoc == chNonBreakSpace)
;	     goto LFCharMatch;
LF05:
	cmp	dl,chNonBreakSpace
	je	Ltemp004
	jmp	short LF11

;	 if (wPat == chHyphen && chDoc == chNonBreakHyphen)
;	     goto LFCharMatch;
LF06:
	cmp	dl,chNonBreakHyphen
	jne	LF12
Ltemp004:
	jmp	LFCharMatch

LF07:
;	 if (!vfSearchCase)
;	     {
;	     if (chDoc == *(pwPat + dwrgwPat))
;		 goto LFCharMatch;
;            if (chDoc > 127)
;                if (ChUpper(chDoc) == wPat)
;                    goto LFCharMatch;
;	     }
	mov	bx,di
	add	bx,[dbrgwPat]
	cmp	dx,[bx]
	je	Ltemp004
        or      dl,dl  
        jns     LF13
	push	ax
	push	cx
	push	dx
	cCall	ChUpper,<dx>
	cmp	ax,[di]
	pop	dx
	pop	cx
	pop	ax
	je	Ltemp004

	jmp	short LF13

LF08:
;	 if (fMatchedWhite)
;	     {	      /* end of matched white space, may still have match    */
;		       /* compare next chPat against SAME chDoc   */
;	     if (--pwPat < pwPatFirst)
;		 {   /* a match has been found	 */
;		 hpchDoc++;	 /* next char was start of white space	 */
;		 goto LFMatchFound;
;		 }
;	     fMatchedWhite = fFalse;
;	     goto LFCheckRemain;
;	     }
	dec	di
	dec	di
	cmp	di,[pwPatFirst]
	jae	LF09
	inc	si
	jmp	LFMatchFound
LF09:
	and	al,NOT maskFMatchedWhiteLocal
	jmp	short LFCheckRemain

	;This code is performed below in the assembler version.
;LFCharLoop:
;
;    if (cchMatched != 0)
;	 {
;	 /* do things the slow way */

LF095:
	pop	ax	;restore flags

;LFSlowCharLoop:
;    /* The character in the document matches if :
;		the search pattern char is chMatchAny
;	     OR the search pattern char is chMatchWhite
;		 AND chDoc is a white-space char
;	     OR chDoc matches the pattern char exactly
;	     OR the search pattern char is a space
;		 AND chDoc is a non-breaking space
;	     OR the search pattern char is a hyphen
;		 AND chDoc is a non-breaking hyphen
;	     OR chDoc is a non-required hyphen
;	     OR the search is NOT case sensitive
;		 AND it matches the pattern char of opposite case
;		 OR (international version only) its corresponding
;		   upper-case char matches the pattern char (to contend
;		   with accented chars: want e(acute) to match E).  */
;	 chDoc = *hpchDoc;
LFSlowCharLoop:
	xor	dx,dx
	mov	dl,es:[si]

;	 if ((wPat = *pwPat) == chDoc)
;	     goto LFCharMatch;
	mov	bx,[di]
	cmp	dx,bx
	je	LFCharMatch

;	 if (chDoc == chNonReqHyphen || chDoc == chReturn)
;	     {
;	     cchMatched++;
;	     goto LFBackUp;
;	     }
	cmp	dl,chNonReqHyphen
	je	LF04
	cmp	dl,chReturn
	je	LF04

;	 if (wPat == wMatchAny && chDoc != chTable)
;	     goto LFCharMatch;
	cmp	bx,wMatchAny
	je	LF025
LF097:

;	 if (wPat == wMatchWhite && FMatchWhiteSpace(chDoc))
;	     {
;	     cchMatched++;
;	     fMatchedWhite = fTrue;
;	     goto LFBackUp;
;	     }
	cmp	bx,wMatchWhite
	je	LF03
LF10:

;	 if (wPat == chSpace && chDoc == chNonBreakSpace)
;	     goto LFCharMatch;
	cmp	bx,chSpace
	je	LF05
LF11:

;	 if (wPat == chHyphen && chDoc == chNonBreakHyphen)
;	     goto LFCharMatch;
	cmp	bx,chHyphen
	je	LF06
LF12:

;	 if (!vfSearchCase)
;	     {
;	     if (chDoc == *(pwPat + dwrgwPat))
;		 goto LFCharMatch;
;            if (chDoc > 127)
;                if (ChUpper(chDoc) == wPat)
;                    goto LFCharMatch;
;	     }
	test	al,maskFSearchCaseLocal
	je	LF07
LF13:

;	 if (fMatchedWhite)
;	     {	      /* end of matched white space, may still have match    */
;		       /* compare next chPat against SAME chDoc   */
;	     if (--pwPat < pwPatFirst)
;		 {   /* a match has been found	 */
;		 hpchDoc++;	 /* next char was start of white space	 */
;		 goto LFMatchFound;
;		 }
;	     fMatchedWhite = fFalse;
;	     goto LFCheckRemain;
;	     }
	test	al,maskFMatchedWhiteLocal
	jne	LF08

;	 fMatchedWhite = fFalse;
	and	al,NOT maskFMatchedWhiteLocal

;	 cchT = max(cchMatched + 1, mpchdcp[chDoc]);
	mov	bx,[mpchdcp]
	add	bx,dx
	mov	dl,[bx]
	mov	bx,[cchMatched]
	inc	bx
	cmp	bx,dx
	jg	LF14
	mov	bx,dx
LF14:

;	 cchMatched = 0;
;	 pwPat = pwPatLast;
;	 hpchDoc += cchT;
;	 ccpRemain -= cchT;
;	 goto LFCheckRemain;
;	 }
	mov	[cchMatched],0
	mov	di,[pwPatLast]
	add	si,bx
	sub	cx,bx
	jmp	short LFCheckRemain

;   else

	;This code is performed above in the C version.
;LFCharLoop:
;
;    if (cchMatched != 0)
;	 {
;	 /* do things the slow way */
LFCharLoop:
	cmp	[cchMatched],0
	jne	LFSlowCharLoop

;	 {
;	 /* do things the fast way */
;	 do
;	     {
	push	ax	  ;save flags
	mov	bx,mpchdcp

;	     chDoc = *hpchDoc;
;	     if ((cchT = mpchdcp[chDoc]) == 0)
;		 goto LFSlowCharLoop;
;	     hpchDoc += cchT;
;	     }
;	 while ((ccpRemain -= cchT) > 0);
;	 }
LF15:
	mov	al,es:[si]
	xlat
	;NOTE: we know ah is zero because there are no flags in ah
	or	ax,ax
	je	LF095
	add	si,ax
	sub	cx,ax
	jg	LF15

	pop	ax	;restore flags

;LFCheckRemain:
;    if (ccpRemain > 0)
;	 goto LFCharLoop;
;    goto LFFetch;
LFCheckRemain:
	or	cx,cx
	jg	LFCharLoop
	mov	[wFlags],ax
	jmp	LFFetch

Ltemp003:
	jmp	LFMatchFound

;LFCharMatch:
;     /* chDoc is a match   */
;    cchMatched++;
;    if (--pwPat < pwPatFirst)
;	 goto LFMatchFound;
LFCharMatch:
	inc	[cchMatched]
	dec	di
	dec	di
	cmp	di,[pwPatFirst]
	jb	Ltemp003

;LFBackUp:
;    hpchDoc--;
;    ccpRemain++;
;    if (ccpRemain <= ccpRun)
;	 goto LFCheckRemain;
LFBackUp:
	dec	si
	inc	cx
	cmp	cx,[ccpRun]
	jle	LFCheckRemain

;	NOTE: from here on down ax is free to be trashed
	mov	[wFlags],ax
;    /* string compare backed over fetch boundary, fetch previous run */
;    ichPat = ((int)pwPat - (int)pwPatFirst) / sizeof(int);
;    ichDoc = max(ichPat - 1, 0);
;    cpRun = vcpFetch - (ichDoc + 1);
	mov	si,di
	sub	si,[pwPatFirst]
	shr	si,1	;ichPat computed
	dec	si
	jge	LF151
	inc	si
LF151:
	mov	ax,si
	neg	ax
	dec	ax
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	[OFF_cpRun],ax
	mov	[SEG_cpRun],dx

;   if (cpRun < cpFirst)
;	{
;	if (pwPat == pwPatFirst && *pwPat == wMatchWhite)
;	    {
;	    hpchDoc = vhpchFetch;
;	    goto LFMatchFound;
;	    }
	sub	ax,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	jge	LF156
	cmp	di,[pwPatFirst]
	jne	LF152
	cmp	[di],wMatchWhite
	jne	LF152
	mov	si,wlo ([vhpchFetch])
	jmp	LFMatchFound

;	 else
LF152:

;	     /* hit cpFirst before matching all of word:  */
;	     {	 /* look beyond the matched string */
;	     ichDoc = cchMatched;
;	     cpRun = vcpFetch;
;	     if (cpFirst == cpFirstInit)
;		 vcpLimWrap = vcpFetch + cchMatched;
;	     pwPat = pwPatLast;
;	     cchMatched = 0;
;	     }
	xor	si,si
	xchg	si,[cchMatched]
	mov	bx,wlo [vcpFetch]
	mov	cx,whi [vcpFetch]
	mov	[OFF_cpRun],bx
	mov	[SEG_cpRun],cx
	mov	ax,[OFF_cpFirst]
	cmp	ax,[OFF_cpFirstInit]
	jne	LF154
	mov	ax,[SEG_cpFirst]
	cmp	ax,[SEG_cpFirstInit]
	jne	LF154
	mov	ax,si
	cwd
	add	ax,bx
	adc	dx,cx
	mov	wlo [vcpLimWrap],ax
	mov	whi [vcpLimWrap],dx
LF154:
	mov	di,[pwPatLast]

;	 }
LF156:

;    FetchCpForSearch(selCur.doc, cpRun, &ccpFetch,
;	     selCur.ww/* fvcScreen*/, pbmib->fNotPlain);
	call	LF345

;    Assert(ccpFetch > 0);
;    Assert(vcpFetch == cpRun); /* there can't have been any
;	 invisible chars skipped because we bumped cpFirst up
;	 past them already */
ifdef DEBUG
;	/* Assert (ccpFetch > 0 && vcpFetch == cpRun) with a call so
;	as not to mess up short jumps */
	call	LF43
endif ;/* DEBUG */

;    goto LFAfterFetch;
	jmp	LFAfterFetch

;LFFetch:
;    ichDoc = (int)hpchDoc - (int)vhpchFetch;
LFFetch:
	sub	si,wlo ([vhpchFetch])

;LFFetchAgain:
;    ichDoc -= ccpRun;
;    cpRun += ccpRun;
LFFetchAgain:
	mov	cx,[ccpRun]
	sub	si,cx
	add	[OFF_cpRun],cx
	adc	[SEG_cpRun],0

;LFFetchAfterReset:
;    /* Need more cp's */
;    Assert (ichDoc >= 0);
LFFetchAfterReset:
ifdef DEBUG
;	/* Assert (ichDoc >= 0) with a call so as not to mess up
;	short jumps */
	call	LF37
endif ;/* DEBUG */

;    if (cpRun + ichDoc >= cpLim)
;	 {
;	 if (fMatchFoundNonText)
;	     goto LFDone;
;	 return cpNil;
;	 }
	mov	ax,si
	cwd
	add	ax,[OFF_cpRun]
	adc	dx,[SEG_cpRun]
	sub	ax,[OFF_cpLim]
	sbb	dx,[SEG_cpLim]
	jl	LF17
LF16:
	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	jne	Ltemp017
LF165:
	mov	ax,LO_cpNil
	errnz	<(LO_cpNil AND HI_cpNil) - 0FFFFh>
	cwd
	jmp	LF33
Ltemp017:
	jmp	LFDone
LF17:

;    /* if any user input (mouse, keyboard) check for abort */
;    if (FQueryAbortCheck())
;	 return cpNil;
;#define FQueryAbortCheck() (vpisPrompt==pisNormal?fFalse:FQueryAbortCheckProc())
	errnz	<pisNormal - 0>
	mov	cx,[vpisPrompt]
	jcxz	LF175
	cCall	FQueryAbortCheckProc,<>
	or	ax,ax
	jne	LF165
LF175:

;    /* report progress */
;    if (hppr != hNil && cpRun >= *pcpNextRpt)
;        ProgressReportPercent(hppr, vprri->cpLowRpt, vprri->cpHighRpt, cpRun, pcpNextRpt);

	cmp	[hppr],0
	je	LF18
	mov	ax,[OFF_cpRun]
	mov	dx,[SEG_cpRun]
	mov	bx,[pcpNextRpt]
	sub	ax,[bx]
	sbb	dx,[bx+2]
	jl	LF18
	push	[hppr]
	xchg	ax,bx
	mov	bx,[vprri]
	push	[bx.HI_cpLowRptRri]
	push	[bx.LO_cpLowRptRri]
	push	[bx.HI_cpHighRptRri]
	push	[bx.LO_cpHighRptRri]
	push	[SEG_cpRun]
	push	[OFF_cpRun]
	push	ax
	cCall	ProgressReportPercent,<>
LF18:

;    FetchCpForSearch(selCur.doc, cpRun, &ccpFetch, selCur.ww/*fvcScreen*/,
;	pbmib->fNotPlain);
	call	LF345

;    if (ccpFetch == 0)
;	 {
;	 if (fMatchFoundNonText)
;	     goto LFDone;
;	 return cpNil;
;	 }
	cmp	[ccpFetch],0
	je	LF16

;    Assert(vcpFetch >= cpRun);
ifdef DEBUG
;	/* Assert (vcpFetch >= cpRun) with a call so as not to mess up
;	short jumps */
	call	LF39
endif ;/* DEBUG */

;    if (vfbSearch.fPap && !FMatchPap() && vcpFetch + ichDoc < cpLim)
;	 {
;	 cpRun = caPara.cpLim;
;	 goto LFResetSearch;
;	 }
	test	[vfbSearch.fPapFb],maskfPapFb
	je	LF22
	cCall	LN_FMatchPap
	jne	LF22
	call	LF34
	jge	LF22
	mov	ax,[caPara.LO_cpLimCa]
	mov	dx,[caPara.HI_cpLimCa]
	;	LFResetSearch expects cpRun in dx:ax
	jmp	LFResetSearch
Ltemp032:
	jmp	LF165

LF22:
;    if (vfbSearch.fChp && !FMatchChp() && vcpFetch + ichDoc < cpLim)
;	 {
;	 cpRun = vcpFetch + ccpFetch;
;	 goto LFResetSearch;
;	 }
	test	[vfbSearch.fChpFb],maskfChpFb
	je	LF225
	cCall	LN_FMatchChp
	jne	LF225
	call	LF34
	jge	LF225
	mov	ax,ccpFetch
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
Ltemp030:
	;	LFResetSearch expects cpRun in dx:ax
	jmp	LFResetSearch

LF225:
;	if (fMatchFoundNonText && (!fTableCur != !vpapFetch.fInTable))
;		goto LFDone;

	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	je	LF227
ifdef DEBUG
	;	Assert that fTableCur = vpapFetch.fInTable or one of them is 0
	call	LF52
endif ;DEBUG
	mov	al,[vpapFetch.fInTablePap]
	cmp	al,bptr [fTableCur]
	je	LF227
	jmp	LFDone
LF227:
	
;    if (vcpFetch > cpRun) /* there were invisible chars skipped */
;	 { /* start over at beginning of pattern */
;	 cpRun = vcpFetch;
;	 goto LFResetSearch;
;	 }
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	mov	cx,[OFF_cpRun]
	sub	cx,ax
	mov	cx,[SEG_cpRun]
	sbb	cx,dx
	;	LFResetSearch expects cpRun in dx:ax
	jl	Ltemp030

;LFAfterFetch:
;    /* Prevent overflow of cchMatched and other variables if
;	someone has a crazy document with a zillion contigous
;	whitespace characters. */
;    if (cchMatched > 0x7FFF - 1 - cbSector)
;	     return cpNil;
LFAfterFetch:
	cmp	[cchMatched],07FFFh-1-cbSector
	ja	Ltemp032

;    Assert(cpLim >= ccpRun);
ifdef DEBUG
;	/* Assert (cpLim >= ccpRun) with a call so as not to mess up
;	short jumps */
	call	LF41
endif ;/* DEBUG */

;    ccpRun = (int)CpMin(cpLim - cpRun, (CP)ccpFetch);
	mov	ax,[OFF_cpLim]
	sub	ax,[OFF_cpRun]
	mov	dx,[SEG_cpLim]
	sbb	dx,[SEG_cpRun]
	mov	cx,[ccpFetch]
	jg	LF23
	cmp	ax,cx
	ja	LF23
	xchg	ax,cx
LF23:
	mov	[ccpRun],cx

;    if (!vfbSearch.fText)
;	 {
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LF25

;	 if (!fMatchFoundNonText)
;	     {
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	jne	LF24

;	     fMatchFoundNonText = fTrue;
;	     fTableCur = vpapFetch.fInTable;
;	     cpMatchStart = vcpFetch;
;	     /* stop at para boundary unless doing non-confirmed replace */
;	     /* allows user to confirm paragraph at a time in a run of
;		 like-formatted paragraphs */
;	     if (!vfReplace || vfConfirm)
;		 cpLim = caPara.cpLim;
;	     }
	or	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	mov	bl,[vpapFetch.fInTablePap]
	mov	bptr [fTableCur],bl
	mov	[caT.LO_cpFirstCa],ax
	mov	[caT.HI_cpFIrstCa],dx
	cmp	[vfReplace],fFalse
	je	LF235
	cmp	[vfConfirm],fFalse
	je	LF24
LF235:
	mov	si,[caPara.LO_cpLimCa]
	mov	[OFF_cpLim],si
	mov	si,[caPara.HI_cpLimCa]
	mov	[SEG_cpLim],si
LF24:

;	 vcpMatchLim = vcpFetch + ccpRun;
;	 ichDoc = ccpRun;
;	 goto LFFetchAgain;
;	 }
	add	ax,cx
	adc	dx,0
	mov	[caT.LO_cpLimCa],ax
	mov	[caT.HI_cpLimCa],dx
	mov	si,cx
	jmp	LFFetchAgain
LF25:

;    ccpRemain = ccpRun - ichDoc;
;    hpchDoc = vhpchFetch + ichDoc;
;    goto LFCheckRemain;
	sub	cx,si
	add	si,wlo ([vhpchFetch])
	mov	ax,[wFlags]
	;LN_SetEsFetch takes sets es according to the sb of vhpchFetch.
	;Only es and bx are altered.
	call	LN_SetEsFetch
	;LFCheckRemain expects ax to contain wFlags,
	;and es to be the ps corresponding to the sb in vhpchFetch.
	jmp	LFCheckRemain

Ltemp013:
	jmp	LFDone

;LFResetSearch:
	;	LFResetSearch expects cpRun in dx:ax
LFResetSearch:
	mov	[OFF_cpRun],ax
	mov	[SEG_cpRun],dx

;    if (fMatchFoundNonText)
;	     goto LFDone;
	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	jne	Ltemp013

;    cpFirst = cpRun;
	mov	[OFF_cpFirst],ax
	mov	[SEG_cpFIrst],dx

;    cpLim = cpLimOrig;
	mov	si,[OFF_cpLimOrig]
	mov	[OFF_cpLim],si
	mov	si,[SEG_cpLimOrig]
	mov	[SEG_cpLim],si

;    ichDoc = pbmib->cwSearch - 1;
	mov	si,[cwSearch]
	dec	si

;    pwPat = pwPatLast;
	mov	di,[pwPatLast]

;    cchMatched = 0;
	mov	[cchMatched],0

;    goto LFFetchAfterReset;
	jmp	LFFetchAfterReset

;LFMatchFound:
;    ichDoc = (int)hpchDoc - (int)vhpchFetch;
;    cpMatchStart = vcpFetch + ichDoc;
;    vcpMatchLim = vcpFetch + ichDoc + cchMatched;
LFMatchFound:
	sub	si,wlo ([vhpchFetch])
	xchg	ax,si
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	[caT.LO_cpFirstCa],ax
	mov	[caT.HI_cpFirstCa],dx
	add	ax,[cchMatched]
	adc	dx,0
	mov	[caT.LO_cpLimCa],ax
	mov	[caT.HI_cpLimCa],dx

;    if (*pwPatLast == wMatchWhite)
;	 {	     /* look for trailing white space characters */
	mov	bx,[pwPatLast]
	cmp	[bx],wMatchWhite
	je	Ltemp002
	jmp	LFDone
Ltemp002:

;	 while (vcpMatchLim < cpLim)
;	     {
LF26:
	mov	ax,[caT.LO_cpLimCa]
	cmp	ax,[OFF_cpLim]
	mov	dx,[caT.HI_cpLimCa]
	mov	bx,dx
	sbb	bx,[SEG_cpLim]
	jl	Ltemp001
LF265:
	jmp	LF32
Ltemp001:

;	     FetchCpForSearch(selCur.doc, vcpMatchLim, &ccpFetch,
;		 selCur.ww/*fvcScreen*/, pbmib->fNotPlain);
	call	LF35

;	     if (ccpFetch == 0)
;		 goto LFDone;
	cmp	[ccpFetch],0
	je	LFDone

;	     Assert(vcpFetch >= vcpMatchLim);
ifdef DEBUG
;	/* Assert (vcpFetch >= vcpMatchLim) with a call so
;	as not to mess up short jumps */
	call	LF46
endif ;/* DEBUG */

;	     if (vcpFetch > vcpMatchLim) /* check for skipped invisible chars */
;		 goto LFDone;
	mov	ax,[caT.LO_cpLimCa]
	sub	ax,wlo [vcpFetch]
	mov	ax,[caT.HI_cpLimCa]
	sbb	ax,whi [vcpFetch]
	jl	LFDone

;	     if (vfbSearch.fChp && !FMatchChp())
;		 goto LFDone;
	test	[vfbSearch.fChpFb],maskFChpFb
	je	LF27
	cCall	LN_FMatchChp
	je	LFDone
 LF27:

;	     if (vfbSearch.fPap && !FMatchPap())
;		 goto LFDone;
	test	[vfbSearch.fPapFb],maskFPapFb
	je	LF28
	cCall	LN_FMatchPap
	je	LFDone

LF28:

;	     Assert(cpLim >= vcpFetch);
ifdef DEBUG
;	/* Assert (cpLim >= vcpFetch) with a call so
;	as not to mess up short jumps */
	call	LF48
endif ;/* DEBUG */

;	     /* Add one to compensate for first decrement below. */
;	     ccpRemain = (int)CpMin(cpLim - vcpFetch, (CP)ccpFetch) + 1;
	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	sub	ax,wlo [vcpFetch]
	sbb	dx,whi [vcpFetch]
	mov	cx,[ccpFetch]
	jg	LF29
	cmp	ax,cx
	ja	LF29
	xchg	cx,ax
LF29:
;	inc	cx	    ;want to use ccpRemain - 1 in loop;

;	     hpchDoc = vhpchFetch;
;	     while (--ccpRemain > 0 && FMatchWhiteSpace(*hpchDoc++));
	mov	si,wlo (vhpchFetch)
	;LN_SetEsFetch takes sets es according to the sb of vhpchFetch.
	;Only es and bx are altered.
	call	LN_SetEsFetch
	jcxz	LF31
LF30:
	lods	byte ptr es:[si]
	xchg	ax,dx
	;LN_FMatchWhiteSpace expects chDoc in dl, and returns equal if true.
	;No registers are changed.
	cCall	LN_FMatchWhiteSpace,<>
	loope	LF30
	jz	LF31
	inc	cx
LF31:

;	     vcpMatchLim = vcpFetch + ((uns)hpchDoc - (uns)vhpchFetch);
	sub	si,wlo ([vhpchFetch])
	xor	dx,dx
	add	si,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]

;	     if (ccpRemain != 0)
;		 {
;		 Assert(ccpRemain > 0);
;		 vcpMatchLim--;
;		 break;
;		 }
	jcxz	LF315
ifdef DEBUG
;	/* Assert (ccpRemain > 0) with a call so
;	as not to mess up short jumps */
	call	LF50
endif ;/* DEBUG */
	sub	si,00001h
	sbb	dx,00000h
LF315:
	mov	[caT.LO_cpLimCa],si
	mov	[caT.HI_cpLimCa],dx
	jcxz	Ltemp014

;	     }
;	 }

LF32:
LFDone:
;    AssureLegalSel(PcaSet(&caT, selCur.doc, cpMatchStart, vcpMatchLim));
	push	[caT.HI_cpFirstCa]
	push	[caT.LO_cpFirstCa]  ;save cpMatchStart
	mov	ax,[selCur.docSel]
	mov	[caT.docCa],ax
	lea	ax,[caT]
	cCall	AssureLegalSel,<ax>

;    /* check for spanning across table cells, not allowed */
;    fTableFirst = FInTableDocCp(caT.doc, caT.cpFirst);
	push	[caT.docCa]
	push	[caT.HI_cpFirstCa]
	push	[caT.LO_cpFirstCa]
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	push	ax	;save fTableFirst

;    fTableLim = FInTableDocCp(caT.doc, caT.cpLim-1);
	mov	ax,[caT.LO_cpLimCa]
	mov	dx,[caT.HI_cpLimCa]
	sub	ax,1
	sbb	dx,0
	push	[caT.docCa]
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	pop	cx	;restore fTableFirst

;    if (fTableFirst || fTableLim)
;	 {
	mov	bx,ax
	or	ax,cx
	pop	ax
	pop	dx	;restore cpMatchStart
	je	LF328

;	 if (!(fTableFirst && fTableLim))
;	     {
	jcxz	LF321
	or	bx,bx
	jnz	LF324
LF321:

;	     cpRun = vfbSearch.fText ? cpMatchStart+1 : caT.cpLim;
	add	ax,00001h
	adc	dx,00000h
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LF322
	mov	ax,[caT.LO_cpLimCa]
	mov	dx,[caT.HI_cpLimCa]
LF322:

;	     fMatchFoundNonText = fFalse;
	and	bptr ([wFlags]),NOT maskfMatchFoundNonTextLocal

;	     goto LFResetSearch;
	;	LFResetSearch expects cpRun in dx:ax
	jmp	LFResetSearch
Ltemp014:
	jmp	LF26

;	     }
LF324:

;	 CacheTc(selCur.ww, caT.doc, caT.cpFirst, fFalse, fFalse);
	errnz	<fFalse>
	push	[selCur.wwSel]
	push	[caT.docCa]
	push	[caT.HI_cpFirstCa]
	push	[caT.LO_cpFirstCa]
	push	si
	push	si
	cCall	CacheTc,<>

;	 if (caT.cpLim >= vtcc.cpLim)
;	     {
	mov	ax,[vtcc.LO_cpLimTcc]
	mov	dx,[vtcc.HI_cpLimTcc]
	cmp	[caT.LO_cpLimCa],ax
	mov	cx,[caT.HI_cpLimCa]
	sbb	cx,dx
	jl	LF328

;	     if (vfbSearch.fText || caT.cpFirst == vtcc.cpLim - ccpEop)
;		 {
;		 cpRun = vtcc.cpLim;
;		 fMatchFoundNonText = fFalse;
;		 goto LFResetSearch;
;		 }
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LF322
	mov	bx,[caT.LO_cpFirstCa]
	mov	cx,[caT.HI_cpFirstCa]
	add	bx,ccpEop
	adc	cx,0
	sub	bx,ax
	sbb	cx,dx
	or	bx,cx
	je	LF322

;	     else
;		 caT.cpLim = vtcc.cpLim - ccpEop;
	sub	ax,ccpEop
	sbb	dx,0
	mov	[caT.LO_cpLimCa],ax
	mov	[caT.HI_cpLimCa],dx

;	     }
;	 }
LF328:

;    cpMatchStart = caT.cpFirst;
;    vcpMatchLim = caT.cpLim;
;    return cpMatchStart;
	lea	si,[caT]
	errnz	<(cpFirstCa) - 0>
	lodsw
	xchg	ax,bx
	lodsw
	xchg	ax,dx
	errnz	<(cpLimCa) - 4>
	lodsw
	mov	wlo [vcpMatchLim],ax
	lodsw
	mov	whi [vcpMatchLim],ax
	xchg	ax,bx

LF33:
;}
cEnd

LF34:
	mov	ax,si
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	sub	ax,[OFF_cpLim]
	sbb	dx,[SEG_cpLim]
	ret

LF345:
	mov	ax,[OFF_cpRun]
	mov	dx,[SEG_cpRun]
LF35:
	;***Begin in-line FetchCpForSearch
;   if (fNotPlain)
;	FetchCpPccpVisible(selCur.doc, cpRun, pccpFetch, ww, fFalse /* fNested */);
;   else
;	{
;	FetchCp(selCur.doc, cpRun, fcmChars);
;	*pccpFetch = vccpFetch;
;	}
	push	[selCur.docSel]
	push	dx
	push	ax
	errnz	<maskFNotPlainLocal - 080h>
	test	bptr ([wFlags]),maskFNotPlainLocal
	je	LF353
	lea	ax,[ccpFetch]
	push	ax
	push	[selCur.wwSel]
	errnz	<fFalse>
	xor	ax,ax
	push	ax
ifdef DEBUG
	cCall	S_FetchCpPccpVisible,<>
else ;not DEBUG
	cCall	N_FetchCpPccpVisible,<>
endif ;DEBUG
	jmp	short LF357
LF353:
	mov	ax,fcmChars
	push	ax
ifdef DEBUG
	cCall	S_FetchCp,<>
else ;not DEBUG
	cCall	N_FetchCp,<>
endif ;DEBUG
	mov	ax,[vccpFetch]
	mov	[ccpFetch],ax
LF357:
	;***End in-line FetchCpForSearch
	ret

;    Assert (ichDoc >= 0);
ifdef DEBUG
LF37:
	cmp	si,0
	jge	LF38
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midSearchn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
LF38:
	ret
endif ;/* DEBUG */

;    Assert(vcpFetch >= cpRun);
ifdef DEBUG
LF39:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	sub	ax,[OFF_cpRun]
	sbb	dx,[SEG_cpRun]
	jge	LF40
	mov	ax,midSearchn
	mov	bx,1003
	cCall	AssertProcForNative,<ax,bx>

LF40:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;    Assert(cpLim >= cpRun);
ifdef DEBUG
LF41:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	sub	ax,[OFF_cpRun]
	sbb	dx,[SEG_cpRun]
	jge	LF42
	mov	ax,midSearchn
	mov	bx,1004
	cCall	AssertProcForNative,<ax,bx>

LF42:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	Assert (ccpFetch > 0 && vcpFetch == cpRun);
ifdef DEBUG
LF43:
	push	ax
	push	bx
	push	cx
	push	dx

	cmp	[ccpFetch],0
	jle	LF44
	mov	ax,wlo [vcpFetch]
	cmp	ax,[OFF_cpRun]
	jne	LF44
	mov	ax,whi [vcpFetch]
	cmp	ax,[SEG_cpRun]
	je	LF45

LF44:
	mov	ax,midSearchn
	mov	bx,1005
	cCall	AssertProcForNative,<ax,bx>

LF45:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	     Assert(vcpFetch >= vcpMatchLim);
ifdef DEBUG
LF46:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	sub	ax,[caT.LO_cpLimCa]
	sbb	dx,[caT.HI_cpLimCa]
	jge	LF47
	mov	ax,midSearchn
	mov	bx,1006
	cCall	AssertProcForNative,<ax,bx>

LF47:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	     Assert(cpLim >= vcpFetch);
ifdef DEBUG
LF48:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	sub	ax,wlo [vcpFetch]
	sbb	dx,whi [vcpFetch]
	jge	LF49
	mov	ax,midSearchn
	mov	bx,1007
	cCall	AssertProcForNative,<ax,bx>

LF49:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	     Assert(ccpRemain > 0);
ifdef DEBUG
LF50:
	push	ax
	push	bx
	push	cx
	push	dx

	or	cx,cx
	jg	LF51
	mov	ax,midSearchn
	mov	bx,1008
	cCall	AssertProcForNative,<ax,bx>

LF51:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

	;	Assert that fTableCur = vpapFetch.fInTable or one of them is 0
ifdef DEBUG
LF52:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	al,[vpapFetch.fInTablePap]
	or	al,al
	je	LF53
	mov	ah,bptr [fTableCur]
	or	ah,ah
	je	LF53
	cmp	ah,al
	je	LF53
	mov	ax,midSearchn
	mov	bx,1040
	cCall	AssertProcForNative,<ax,bx>

LF53:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	CpSearchSzBackward(pbmib, cpFirst, cpLim)
;-------------------------------------------------------------------------
;NATIVE CP CpSearchSzBackward (pbmib, cpFirst, cpLim)
;struct BMIB *pbmib;
;CP    cpFirst;
;CP    cpLim;
;{
;/*  *** CpSearchSzBackward ***
;
;    Same as CpSearchSz, but searches backward.  Uses the Boyer/Moore
;    algorithm in reverse.
;
;    Returns the first cp of the text match in the document; cpNil if
;    not found.
;  */
;
;    int    *pwPatFirst = pbmib->rgwSearch;  /* local for speed */
;    char   *mpchdcp = pbmib->mpchdcp;	     /* local for speed */
;    int    chDoc;
;    char    HUGE *hpchDoc;
;    BOOL    fMatchedWhite;
;    int cchMatched;
;    CP    cpRun, cpMatchStart;
;    int ccpFetch;		  /* local instead of vccpFetch */
;    int    ichDoc, cchT;
;    int    wPat, *pwPat;
;    int    ccpRun, ccpRemain;
;    int dwrgwPat;
;    int ichPat;
;    int *pwPatLast;
;    CP     cpLimInit = cpLim;
;    BOOL   fMatchFoundNonText = fFalse;
;    struct CA caT;
;    int    icpFetchMac;	    /* number of entries in rgcpFetch */
;    CP    rgcpFetch[128];    /* record of beginnings of visible runs */
;    BOOL   fTableFirst, fTableLim;
;    CP     cpFirstOrig = cpFirst;

; %%Function:N_CpSearchSzBackward %%Owner:BRADV
cProc	N_CpSearchSzBackward,<PUBLIC,FAR>,<si,di>
	ParmW	<pbmib>
	ParmD	<cpFirst>
	ParmD	<cpLim>

	LocalW	pwPatFirst
	LocalW	mpchdcp
	LocalW	cchMatched
	LocalD	cpRun
	LocalW	ccpFetch
	LocalW	ccpRun
	LocalW	dbrgwPat
	LocalW	pwPatLast
	LocalD	cpLimInit
	LocalW	cwSearch
	LocalW	wFlags
	LocalD	cpFirstOrig
	LocalV	caT,cbCaMin

;The following variables are used by subroutines of CpSearchSzBackward
	LocalD	cpLimRun
	LocalD	cpCur
	LocalW	fRunStartFetched
ifdef DEBUG
	LocalW	cBeenHere
	LocalD	cpFetchSav
endif ;/* DEBUG */
	LocalW	icpFetchMac
	LocalV	rgcpFetch, 128*4

cBegin

	mov	bx,[pbmib]

;    Assert (!pbmib->fNotPlain || !pbmib->fChTableInPattern);
ifdef DEBUG
	cmp	[bx.fNotPlainBmib],fFalse
	je	LB005
	cmp	[bx.fChTableInPatternBmib],fFalse
	je	LB005
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midSearchn
	mov	bx,1033
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LB005:
endif ;/* DEBUG */

	errnz	<(rgwSearchBmib) - 0>
	mov	[pwPatFirst],bx
	lea	ax,[bx.mpchdcpBmib]
	mov	[mpchdcp],ax

;    dwrgwPat = ((int)pbmib->rgwOppCase - (int)pwPatFirst) / sizeof(int);
	mov	dbrgwPat,(rgwOppCaseBmib)-(rgwSearchBmib)

;    fMatchedWhite = fFalse;
;    fMatchFoundNonText = fFalse;
	;simultaneously put vfSearchCase in register
	;and clear fMatchedWhite, fMatchFoundNonText
	mov	ax,vfSearchCase
ifdef DEBUG
;	Make sure vfSearchCase exists only in the low bit.
	test	ax,NOT maskFSearchCaseLocal
	je	LB01
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midSearchn
	mov	bx,1009
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
LB01:
endif ;/* DEBUG */
ifdef DEBUG
;	Make sure pbmib->fNotPlain exists only in the low bit.
	test	[bx.fNotPlainBmib],0FFFEh
	je	LB015
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midSearchn
	mov	bx,1031
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
LB015:
endif ;/* DEBUG */
	errnz	<maskFNotPlainLocal - 080h>
	mov	cl,bptr ([bx.fNotPlainBmib])
	ror	cl,1
	or	al,cl
	mov	[wFlags],ax
	mov	ax,[bx.cwSearchBmib]
	mov	[cwSearch],ax

;    pwPatLast = &pwPatFirst[pbmib->cwSearch - 1];
	dec	ax
	errnz	<(rgwSearchBmib) - 0>
	add	bx,ax
	add	bx,ax
	mov	[pwPatLast],bx

	mov	si,[OFF_cpFirst]
	mov	di,[SEG_cpFirst]
	;frame initialization of cpFirstOrig done here
	mov	[OFF_cpFirstOrig],si
	mov	[SEG_cpFirstOrig],di

;    vcpLimWrap = CpMax(cpLim - pbmib->cwSearch + 1, cpFirst);
	neg	ax
	cwd
	add	ax,[OFF_cpLim]
	adc	dx,[SEG_cpLim]
	mov	bx,ax
	mov	cx,dx
	sub	bx,si
	sbb	cx,di
	jge	LB02
	mov	ax,si
	mov	dx,di
LB02:
	mov	wlo [vcpLimWrap],ax
	mov	whi [vcpLimWrap],dx

;    cpRun = cpLim;
	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	;frame initialization of cpLimInit also done here
	mov	[OFF_cpLimInit],ax
	mov	[SEG_cpLimInit],dx

;    goto LBResetSearch;
	;	LBResetSearch expects cpRun in dx:ax
	jmp	LBResetSearch

;	 if (wPat == wMatchAny && chDoc != chTable)
;	     goto LBCharMatch;
LB025:
	cmp	dl,chTable
	je	LB097
	jmp	short Ltemp010

LB03:
;	 if (wPat == wMatchWhite && FMatchWhiteSpace(chDoc))
;	     {
;	     cchMatched++;
;	     fMatchedWhite = fTrue;
;	     goto LBBackUp;
;	     }
	;LN_FMatchWhiteSpace expects chDoc in dl, and returns equal if true.
	;No registers are changed.
	cCall	LN_FMatchWhiteSpace,<>
	jne	LB10
	or	al,maskfMatchedWhiteLocal
LB04:
	inc	cchMatched;
	jmp	LBBackUp

;	 if (wPat == chSpace && chDoc == chNonBreakSpace)
;	     goto LBCharMatch;
LB05:
	cmp	dl,chNonBreakSpace
	je	Ltemp010
	jmp	short LB11

;	 if (wPat == chHyphen && chDoc == chNonBreakHyphen)
;	     goto LBCharMatch;
LB06:
	cmp	dl,chNonBreakHyphen
	jne	LB12
Ltemp010:
	jmp	LBCharMatch

LB07:
;	 if (!vfSearchCase)
;	     {
;	     if (chDoc == *(pwPat + dwrgwPat))
;		 goto LBCharMatch;
;            if (chDoc > 127)
;                if (ChUpper(chDoc) == wPat)
;                    goto LBCharMatch;
;	     }
	mov	bx,di
	add	bx,[dbrgwPat]
	cmp	dx,[bx]
	je	Ltemp010
        or      dl,dl
        jns     LB13
	push	ax
	push	cx
	push	dx
	cCall	ChUpper,<dx>
	cmp	ax,[di]
	pop	dx
	pop	cx
	pop	ax
	je	Ltemp010
	jmp	short LB13

Ltemp009:
	jmp	LBMatchFound

LB08:
;	 if (fMatchedWhite)
;	     {	      /* end of matched white space, may still have match    */
;		       /* compare next chPat against SAME chDoc   */
;	     if (++pwPat > pwPatLast)
;		 {    /* a match has been found */
;		 goto LBMatchFound;
;		 }
;	     fMatchedWhite = fFalse;
;	     goto LBCheckRemain;
;	     }
	inc	di
	inc	di
	cmp	di,[pwPatLast]
	ja	Ltemp009
	and	al,NOT maskFMatchedWhiteLocal
	jmp	short LBCheckRemain

	;This code is performed below in the assembler version.
;LBCharLoop:
;    if (cchMatched != 0)
;	 {
;	 /* do things the slow way */

LB095:
	pop	ax	;restore flags

;LBSlowCharLoop:
;    /* The character in the document matches if :
;		the search pattern char is chMatchAny
;	     OR the search pattern char is chMatchWhite
;		 AND chDoc is a white-space char
;	     OR chDoc matches the pattern char exactly
;	     OR the search pattern char is a space
;		 AND chDoc is a non-breaking space
;	     OR the search pattern char is a hyphen
;		 AND chDoc is a non-breaking hyphen
;	     OR chDoc is a non-required hyphen
;	     OR the search is NOT case sensitive
;		 AND it matches the pattern char of opposite case
;		 OR (international version only) its corresponding
;		   upper-case char matches the pattern char (to contend
;		   with accented chars: want e(acute) to match E).  */
;	 chDoc = *hpchDoc;
LBSlowCharLoop:
	xor	dx,dx
	mov	dl,es:[si]

;	 if ((wPat = *pwPat) == chDoc)
;	     goto LBCharMatch;
	mov	bx,[di]
	cmp	dx,bx
	je	LBCharMatch

;	 if (chDoc == chNonReqHyphen || chDoc == chReturn)
;	     {
;	     cchMatched++;
;	     goto LBBackUp;
;	     }
	cmp	dl,chNonReqHyphen
	je	LB04
	cmp	dl,chReturn
	je	LB04

;	 if (wPat == wMatchAny && chDoc != chTable)
;	     goto LBCharMatch;
	cmp	bx,wMatchAny
	je	LB025
LB097:

;	 if (wPat == wMatchWhite && FMatchWhiteSpace(chDoc))
;	     {
;	     cchMatched++;
;	     fMatchedWhite = fTrue;
;	     goto LBBackUp;
;	     }
	cmp	bx,wMatchWhite
	je	LB03
LB10:

;	 if (wPat == chSpace && chDoc == chNonBreakSpace)
;	     goto LBCharMatch;
	cmp	bx,chSpace
	je	LB05
LB11:

;	 if (wPat == chHyphen && chDoc == chNonBreakHyphen)
;	     goto LBCharMatch;
	cmp	bx,chHyphen
	je	LB06
LB12:

;	 if (!vfSearchCase)
;	     {
;	     if (chDoc == *(pwPat + dwrgwPat))
;		 goto LBCharMatch;
;	     if (chDoc > 127)
;               if (ChUpper(chDoc) == wPat)
;		     goto LBCharMatch;
;	     }
	test	al,maskFSearchCaseLocal
	je	LB07
LB13:

;	 if (fMatchedWhite)
;	     {	      /* end of matched white space, may still have match    */
;		       /* compare next chPat against SAME chDoc   */
;	     if (++pwPat > pwPatLast)
;		 {   /* a match has been found	 */
;		 goto LBMatchFound;
;		 }
;	     fMatchedWhite = fFalse;
;	     goto LBCheckRemain;
;	     }
	test	al,maskFMatchedWhiteLocal
	jne	LB08

;	 fMatchedWhite = fFalse;
	and	al,NOT maskFMatchedWhiteLocal

;	 cchT = max(cchMatched + 1, mpchdcp[chDoc]);
	mov	bx,[mpchdcp]
	add	bx,dx
	mov	dl,[bx]
	mov	bx,[cchMatched]
	inc	bx
	cmp	bx,dx
	jg	LB14
	mov	bx,dx
LB14:

;	 cchMatched = 0;
;	 pwPat = pwPatFirst;
;	 hpchDoc -= cchT;
;	 ccpRemain -= cchT;
;	 goto LBCheckRemain;
;	 }
	mov	[cchMatched],0
	mov	di,[pwPatFirst]
	sub	si,bx
	sub	cx,bx
	jmp	short LBCheckRemain

;   else

	;This code is performed above in the C version.
;LBCharLoop:
;
;    if (cchMatched != 0)
;	 {
;	 /* do things the slow way */
LBCharLoop:
	cmp	[cchMatched],0
	jne	LBSlowCharLoop

;	 {
;	 /* do things the fast way */
;	 do
;	     {
	push	ax	  ;save flags
	mov	bx,[mpchdcp]

;	     chDoc = *hpchDoc;
;	     if ((cchT = mpchdcp[chDoc]) == 0)
;		 goto LBSlowCharLoop;
;	     hpchDoc -= cchT;
;	     }
;	 while ((ccpRemain -= cchT) > 0);
;	 }
LB15:
	mov	al,es:[si]
	xlat
	;NOTE: we know ah is zero because there are no flags in ah
	or	ax,ax
	je	LB095
	sub	si,ax
	sub	cx,ax
	jg	LB15

	pop	ax	;restore flags

;LBCheckRemain:
;    if (ccpRemain > 0)
;	 goto LBCharLoop;
;    goto LBFetch;
LBCheckRemain:
	or	cx,cx
	jg	LBCharLoop
	mov	[wFlags],ax
	jmp	LBFetch

Ltemp005:
	jmp	LBMatchFound

;LBCharMatch:
;    /* chDoc is a match */
;    cchMatched++;
;    if (++pwPat > pwPatLast)
;	 {
;	 hpchDoc++;    /* advance pch to get the Lim */
;	 goto LBMatchFound;
;	 }
LBCharMatch:
	inc	[cchMatched]
	inc	di
	inc	di
	inc	si
	cmp	di,[pwPatLast]
	ja	Ltemp005
	dec	si

;LBBackUp:
;    hpchDoc++;
;    ccpRemain++;
;    if (ccpRemain <= ccpRun)
;	 goto LBCheckRemain;
LBBackUp:
	inc	si
	inc	cx
	cmp	cx,[ccpRun]
	jle	LBCheckRemain

;	NOTE: from here on down ax is free to be trashed
	mov	[wFlags],ax
;    /* string compare advanced over fetch boundary, fetch next run */
;    ichPat = ((int)pwPat - (int)pwPatFirst) / sizeof(int);
;    cpRun = vcpFetch + ccpFetch;
;    ichDoc = 0;
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	[OFF_cpRun],ax
	mov	[SEG_cpRun],dx
	mov	cx,[cwSearch]
	mov	si,di
	sub	si,[pwPatFirst]
	shr	si,1
	sub	cx,si
	xor	si,si	;ichDoc computed

;    if (cpRun + (pbmib->cwSearch - ichPat) > cpLim)
;	 {
;	 if (pwPat == pwPatLast && *pwPat == wMatchWhite)
;	     {
;	     hpchDoc = vhpchFetch;
;	     goto LBMatchFound;
;	     }
	add	ax,cx
	adc	dx,0
	mov	cx,[OFF_cpLim]
	sub	cx,ax
	mov	cx,[SEG_cpLim]
	sbb	cx,dx
	jge	LB156
	cmp	di,[pwPatLast]
	jne	LB152
	cmp	[di],wMatchWhite
	jne	LB152
	mov	si,wlo ([vhpchFetch])
	jmp	LBMatchFound

;	 else
LB152:

;	     /* hit cpLim before matching all of word:	*/
;	     {	/* try starting 1 char earlier in doc */
;	     /* We must invalidate rgcpFetch here because we have
;	     no guarantee that we will be fetching the piece just after
;	     the one starting at rgcpFetch[icpFetchMac - 1]. */
;	     icpFetchMac = 0;
;	     if (cpLim == cpLimInit)
;		 vcpLimWrap = cpRun - cchMatched;
;	     ichDoc = -cchMatched - 1;
;	     pwPat = pwPatFirst;
;	     cchMatched = 0;
;	     goto LBFetchAfterReset;
;	     }
	xor	si,si
	mov	[icpFetchMac],si
	xchg	si,[cchMatched]
	neg	si
	mov	ax,[OFF_cpLim]
	cmp	ax,[OFF_cpLimInit]
	jne	LB154
	mov	ax,[SEG_cpLim]
	cmp	ax,[SEG_cpLimInit]
	jne	LB154
	mov	ax,si
	cwd
	add	ax,[OFF_cpRun]
	adc	dx,[SEG_cpRun]
	mov	wlo [vcpLimWrap],ax
	mov	whi [vcpLimWrap],dx
LB154:
	dec	si
	mov	di,[pwPatFirst]
	jmp	short LBFetchAfterReset

;	 }
LB156:

;    FetchCpForSearch(selCur.doc, cpRun, &ccpFetch,
;	 selCur.ww/*fvcScreen*/, fFalse);
	call	LB345

;    Assert(ccpFetch > 0);
;    Assert(vcpFetch == cpRun);    /* there can't have been any
;	 invisible chars skipped because we bumped cpLim down past them
;	 already. */
;    Assert(icpFetchMac >= 0);
;    Debug(CheckRgcpFetch(selCur.ww, &ccpFetch, pbmib->fNotPlain,
;	 &icpFetchMac, rgcpFetch));
ifdef DEBUG
;	/* Do this debug code with a call so as not to mess up
;	short jumps */
	call	LB39
endif ;/* DEBUG */

;    if ((long)ccpFetch > cpLim - vcpFetch)
;	 ccpFetch = (int)(cpLim - vcpFetch);
	mov	ax,[OFF_cpLim]
	mov	dx,[SEG_cpLim]
	sub	ax,wlo [vcpFetch]
	sbb	dx,whi [vcpFetch]
	jg	LB158
	cmp	ax,[ccpFetch]
	jae	LB158
	mov	[ccpFetch],ax
LB158:

;    if (icpFetchMac < 128)
;	 rgcpFetch[icpFetchMac++] = vcpFetch;
;    else
;	 {
;	 Assert(icpFetchMac == 128);
;	 blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;	 rgcpFetch[128 - 1] = cpRun;
;	 }

;	LB36 performs:
;	if (icpFetchMac < 128)
;	    rgcpFetch[icpFetchMac++] = vcpFetch;
;	else
;	    {
;	    Assert(icpFetchMac == 128);
;	    blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;	    rgcpFetch[128 - 1] = vcpFetch;
;	    }
;	registers ax, bx, cx are altered
	call	LB36

;	 }
;    goto LBAfterFetch;
	jmp	LBAfterFetch

;LBFetch:
;    ichDoc = (int)hpchDoc - (int)vhpchFetch;
LBFetch:
	sub	si,wlo ([vhpchFetch])

;LBFetchAgain:
;LBFetchAfterReset:
;    /* Need more cp's */
;    Assert (ichDoc < 0);
LBFetchAgain:
LBFetchAfterReset:

;    if (cpRun + ichDoc < cpFirst)
;	 {
;	 if (fMatchFoundNonText)
;	     goto LBDone;
;	 return cpNil;
;	 }
	mov	ax,si
	cwd
	add	ax,[OFF_cpRun]
	adc	dx,[SEG_cpRun]
	sub	ax,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	jge	LB17
LB16:
	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	jne	Ltemp018
LB165:
	mov	ax,LO_cpNil
	errnz	<(LO_cpNil AND HI_cpNil) - 0FFFFh>
	cwd
	jmp	LB33
Ltemp018:
	jmp	LBDone
LB17:

ifdef DEBUG
;	/* Assert (ichDoc < 0) with a call so as not to mess up
;	short jumps */
	call	LB37
endif ;/* DEBUG */

;    /* if any user input (mouse, keyboard) check for abort */
;    if (FQueryAbortCheck())
;	 return cpNil;
;#define FQueryAbortCheck() (vpisPrompt==pisNormal?fFalse:FQueryAbortCheckProc())
	errnz	<pisNormal - 0>
	mov	cx,[vpisPrompt]
	jcxz	LB173
	cCall	FQueryAbortCheckProc,<>
	or	ax,ax
	jne	LB165
LB173:

;    FetchCpPccpVisibleBackward(selCur.doc, cpRun,
;	 &ccpFetch, selCur.ww, pbmib->fNotPlain, &icpFetchMac, rgcpFetch);
	mov	ax,[OFF_cpRun]
	mov	dx,[SEG_cpRun]
	;LN_FetchCpPccpVisibleBackward uses its caller's frame.  It takes
	;the CP at which to begin searching from dx:ax.  si is not altered.
	cCall	LN_FetchCpPccpVisibleBackward,<>

;    if (ccpFetch == 0)
;	 {
;	 if (fMatchFoundNonText)
;	     return cpMatchStart;
;	 return cpNil;
;	 }
	cmp	[ccpFetch],0
	je	LB16

;    if ((long)ccpFetch > cpRun - vcpFetch)
;	 ccpFetch = (int)(cpRun - vcpFetch);
	mov	ax,[OFF_cpRun]
	mov	dx,[SEG_cpRun]
	sub	ax,wlo [vcpFetch]
	sbb	dx,whi [vcpFetch]
	jg	LB175
	cmp	ax,[ccpFetch]
	ja	LB175
	mov	[ccpFetch],ax
LB175:

;    ichDoc += (int)(cpRun - vcpFetch);
	add	si,ax

;    if (vfbSearch.fPap && !FMatchPap() && vcpFetch + ichDoc >= cpFirst)
;	 {
;	 cpRun = caPara.cpFirst;
;	 goto LBResetSearch;
;	 }
	test	[vfbSearch.fPapFb],maskfPapFb
	je	LB22
	cCall	LN_FMatchPap
	jne	LB22
	call	LB34
	jl	LB22
	mov	ax,[caPara.LO_cpFirstCa]
	mov	dx,[caPara.HI_cpFirstCa]
	;	LBResetSearch expects cpRun in dx:ax
	jmp	LBResetSearch
Ltemp033:
	jmp	short LB165

LB22:
;    if (vfbSearch.fChp && !FMatchChp() && vcpFetch + ichDoc >= cpFirst)
;	 {
;	 cpRun = vcpFetch;
;	 goto LBResetSearch;
;	 }
	test	[vfbSearch.fChpFb],maskfChpFb
	je	LB225
	cCall	LN_FMatchChp
	jne	LB225
	call	LB34
	jl	LB225
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	;	LBResetSearch expects cpRun in dx:ax
Ltemp019:
	jmp	LBResetSearch

LB225:
;    if (vcpFetch + ccpFetch < cpRun) /* there were invisible chars skipped */
;	 { /* start from the beginning of pattern */
;	 cpRun = vcpFetch + ccpFetch;
;	 goto LBResetSearch;
;	 }
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	cx,ax
	sub	cx,[OFF_cpRun]
	mov	cx,dx
	sbb	cx,[SEG_cpRun]
	;	LBResetSearch expects cpRun in dx:ax
	jl	Ltemp019

;    cpRun = vcpFetch;
	mov	ax,wlo [vcpFetch]
	mov	[OFF_cpRun],ax
	mov	ax,whi [vcpFetch]
	mov	[SEG_cpRun],ax

;LBAfterFetch:
;    /* Prevent overflow of cchMatched and other variables if
;	someone has a crazy document with a zillion contigous
;	whitespace characters. */
;    if (cchMatched > 0x7FFF - 1 - cbSector)
;	     return cpNil;
;    Assert(vcpFetch + ccpFetch > cpFirst);
LBAfterFetch:
	cmp	[cchMatched],07FFFh-1-cbSector
	ja	Ltemp033
ifdef DEBUG
;	/* Assert (vcpFetch + ccpFetch > cpFirst) with a call so as
;	not to mess up short jumps */
	call	LB41
endif ;/* DEBUG */

;    ccpRun = (int)CpMin(vcpFetch + ccpFetch - cpFirst, (CP)ccpFetch);
	mov	cx,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	sub	cx,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	jl	LB23
	xor	cx,cx
LB23:
	add	cx,[ccpFetch]
	mov	[ccpRun],cx

;    if (!vfbSearch.fText)
;	 {
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LB25

;	 if (!fMatchFoundNonText)
;	     {
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	jne	LB24

;	     fMatchFoundNonText = fTrue;
;	     vcpMatchLim = vcpFetch + ccpFetch;
;	     /* stop at para boundary unless doing non-confirmed replace */
;	     /* allows user to confirm paragraph at a time in a run of
;		 like-formatted paragraphs */
;	     if (!vfReplace || vfConfirm)
;		 cpFirst = caPara.cpFirst;
;	     }
	or	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	mov	[caT.LO_cpLimCa],ax
	mov	[caT.HI_cpLimCa],dx
	cmp	[vfReplace],fFalse
	je	LB235
	cmp	[vfConfirm],fFalse
	je	LB24
LB235:
	mov	si,[caPara.LO_cpFirstCa]
	mov	[OFF_cpFirst],si
	mov	si,[caPara.HI_cpFirstCa]
	mov	[SEG_cpFirst],si
LB24:

;	 cpMatchStart = vcpFetch + ccpFetch - ccpRun;
;	 ichDoc = -1;
;	 goto LBFetchAgain;
;	 }
	sub	ax,cx
	sbb	dx,0
	mov	[caT.LO_cpFirstCa],ax
	mov	[caT.HI_cpFirstCa],dx
	mov	si,-1
	jmp	LBFetchAgain
LB25:

;    /* Add one here in the backward case because we want to include
;    the character at cpFirst in the characters we look at.  In the
;    forward case we do not want to include the character at cpLim. */
;    ccpRemain = ichDoc - (ccpFetch - ccpRun) + 1;
;    hpchDoc = vhpchFetch + ichDoc;
;    goto LBCheckRemain;
	add	cx,si
	sub	cx,[ccpFetch]
	inc	cx
	add	si,wlo ([vhpchFetch])
	mov	ax,[wFlags]
	;LN_SetEsFetch takes sets es according to the sb of vhpchFetch.
	;Only es and bx are altered.
	call	LN_SetEsFetch
	;LBCheckRemain expects ax to contain wFlags,
	;and es to be the ps corresponding to the sb in vhpchFetch.
	jmp	LBCheckRemain

Ltemp012:
	jmp	LBDone

;LBResetSearch:
;    /* We might as well invalidate rgcpFetch since we know anything
;    in it will be useless. */
	;	LBResetSearch expects cpRun in dx:ax
LBResetSearch:
	mov	[OFF_cpRun],ax
	mov	[SEG_cpRun],dx

;    if (fMatchFoundNonText)
;	 goto LBDone;
	test	bptr ([wFlags]),maskfMatchFoundNonTextLocal
	jne	Ltemp012

;    icpFetchMac = 0;
	mov	[icpFetchMac],0

;    cpLim = cpRun;
	mov	[OFF_cpLim],ax
	mov	[SEG_cpLim],dx

;    cpFirst = cpFirstOrig;
	mov	si,[OFF_cpFirstOrig]
	mov	[OFF_cpFirst],si
	mov	si,[SEG_cpFirstOrig]
	mov	[SEG_cpFirst],si

;    ichDoc = -pbmib->cwSearch;
	mov	si,[cwSearch]
	neg	si

;    pwPat = pwPatFirst;
	mov	di,[pwPatFirst]

;    cchMatched = 0;
	mov	[cchMatched],0

;    goto LBFetchAfterReset;
	jmp	LBFetchAfterReset

;LBMatchFound:
;    ichDoc = (int)hpchDoc - (int)vhpchFetch;
;    cpMatchStart = cpRun + ichDoc - cchMatched;
;    vcpMatchLim = cpRun + ichDoc;
LBMatchFound:
	sub	si,wlo ([vhpchFetch])
	xchg	ax,si
	cwd
	add	ax,[OFF_cpRun]
	adc	dx,[SEG_cpRun]
	mov	[caT.LO_cpLimCa],ax
	mov	[caT.HI_cpLimCa],dx
	sub	ax,[cchMatched]
	sbb	dx,0
	mov	[caT.LO_cpFirstCa],ax
	mov	[caT.HI_cpFirstCa],dx

;    if (*pwPatFirst == wMatchWhite)
;	 {	     /* look for leading white space characters   */
	mov	bx,[pwPatFirst]
	cmp	[bx],wMatchWhite
	je	Ltemp008
Ltemp016:
	jmp	LBDone
Ltemp008:

;	 while (cpMatchStart > cpFirst)
;	     {
LB26:
	mov	ax,[caT.LO_cpFirstCa]
	mov	cx,[OFF_cpFirst]
	sub	cx,ax
	mov	dx,[caT.HI_cpFirstCa]
	mov	cx,[SEG_cpFirst]
	sbb	cx,dx
	jl	Ltemp011
LB265:
	jmp	LB32
Ltemp011:

;	     FetchCpPccpVisibleBackward(selCur.doc, cpMatchStart,
;		 &ccpFetch, selCur.ww, pbmib->fNotPlain, &icpFetchMac, rgcpFetch);
	;LN_FetchCpPccpVisibleBackward uses its caller's frame.  It takes
	;the CP at which to begin searching from dx:ax.  si is not altered.
	cCall	LN_FetchCpPccpVisibleBackward,<>

;	     if (ccpFetch == 0)
;		 goto LBDone;
	cmp	[ccpFetch],0
	je	Ltemp016

;	     /* check for skipped invisible chars */
;	     if (vcpFetch + ccpFetch < cpMatchStart)
;		 goto LBDone;
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	sub	ax,[caT.LO_cpFirstCa]
	sbb	dx,[caT.HI_cpFirstCa]
	jl	LBDone

;	     if (vfbSearch.fChp && !FMatchChp())
;		 goto LBDone;
	test	[vfbSearch.fChpFb],maskFChpFb
	je	LB27
	cCall	LN_FMatchChp
	je	LBDone
LB27:

;	     if (vfbSearch.fPap && !FMatchPap())
;		 goto LBDone;
	test	[vfbSearch.fPapFb],maskFPapFb
	je	LB28
	cCall	LN_FMatchPap
	je	LBDone

LB28:

;	     /* Add one to compensate for first decrement below. */
;	     ccpRemain = (int)(cpMatchStart - CpMax(cpFirst, vcpFetch)) + 1;
	mov	ax,wlo [vcpFetch]
	mov	bx,[OFF_cpFirst]
	mov	cx,ax
	sub	cx,bx
	mov	cx,whi [vcpFetch]
	sbb	cx,[SEG_cpFirst]
	jl	LB29
	mov	bx,ax
LB29:
	mov	si,[caT.LO_cpFirstCa]
	mov	cx,si
	sub	cx,bx
;	inc	cx	    ;want to use ccpRemain - 1 in loop;

;	     hpchDoc = vhpchFetch + (int)(cpMatchStart - vcpFetch);
;	     while (--ccpRemain > 0 && FMatchWhiteSpace(*(--hpchDoc)));
	sub	si,ax
	add	si,wlo ([vhpchFetch])
	;LN_SetEsFetch takes sets es according to the sb of vhpchFetch.
	;Only es and bx are altered.
	call	LN_SetEsFetch
	dec	si	    ;adjust for lodsb
	std
	jcxz	LB31
LB30:
	lods	byte ptr es:[si]
	xchg	ax,dx
	;LN_FMatchWhiteSpace expects chDoc in dl, and returns equal if true.
	;No registers are changed.
	cCall	LN_FMatchWhiteSpace,<>
	loope	LB30
	jz	LB31
	inc	cx
LB31:
	cld
	inc	si	    ;adjust for lodsb

;	     cpMatchStart = vcpFetch
;		 + ((uns)hpchDoc - (uns)vhpchFetch);
	sub	si,wlo ([vhpchFetch])
	xor	dx,dx
	add	si,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]

;	     if (ccpRemain != 0)
;		 {
;		 Assert(ccpRemain > 0);
;		 cpMatchStart++;
;		 break;
;		 }
;	     }
;	 }
	jcxz	LB315
ifdef DEBUG
;	/* Assert (ccpRemain > 0) with a call so
;	as not to mess up short jumps */
	call	LB50
endif ;/* DEBUG */
	add	si,00001h
	adc	dx,00000h
LB315:
	mov	[caT.LO_cpFirstCa],si
	mov	[caT.HI_cpFirstCa],dx
	jcxz	Ltemp015

LB32:
LBDone:
;    AssureLegalSel(PcaSet(&caT, selCur.doc, cpMatchStart, vcpMatchLim));
	push	[caT.HI_cpLimCa]
	push	[caT.LO_cpLimCa]  ;save vcpMatchLim
	mov	ax,[selCur.docSel]
	mov	[caT.docCa],ax
	lea	ax,[caT]
	cCall	AssureLegalSel,<ax>

;    /* check for spanning across table cells, not allowed */
;    fTableFirst = FInTableDocCp(caT.doc, caT.cpFirst);
	push	[caT.docCa]
	push	[caT.HI_cpFirstCa]
	push	[caT.LO_cpFirstCa]
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	push	ax	;save fTableFirst

;    fTableLim = FInTableDocCp(caT.doc, caT.cpLim-1, NULL);
	mov	ax,[caT.LO_cpLimCa]
	mov	dx,[caT.HI_cpLimCa]
	sub	ax,1
	sbb	dx,0
	push	[caT.docCa]
	push	dx
	push	ax
ifdef DEBUG
	cCall	S_FInTableDocCp,<>
else ;not DEBUG
	cCall	N_FInTableDocCp,<>
endif ;DEBUG
	pop	cx	;restore fTableFirst

;    if (fTableFirst || fTableLim)
;	 {
	mov	bx,ax
	or	ax,cx
	pop	ax
	pop	dx	;restore vcpMatchLim
	je	Ltemp020

;	 if (!(fTableFirst && fTableLim))
;	     {
	jcxz	LB321
	or	bx,bx
	jnz	LB324
LB321:

;	     cpRun = vfbSearch.fText ? vcpMatchLim-1 : caT.cpFirst;
	sub	ax,00001h
	sbb	dx,00000h
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LB322
	mov	ax,[caT.LO_cpFirstCa]
	mov	dx,[caT.HI_cpFirstCa]
LB322:

;	     fMatchFoundNonText = fFalse;
	and	bptr ([wFlags]),NOT maskfMatchFoundNonTextLocal

;	     goto LBResetSearch;
	;	LBResetSearch expects cpRun in dx:ax
	jmp	LBResetSearch
Ltemp015:
	jmp	LB26
Ltemp020:
	jmp	short LB328

;	     }
LB324:

;	 CacheTc(selCur.ww, caT.doc, caT.cpFirst, fFalse, fFalse);
	errnz	<fFalse>
	push	[selCur.wwSel]
	push	[caT.docCa]
	push	[caT.HI_cpFirstCa]
	push	[caT.LO_cpFirstCa]
	push	si
	push	si
	cCall	CacheTc,<>

;	 if (caT.cpFirst < vtcc.cpFirst)
;	     {
	mov	ax,[vtcc.LO_cpFirstTcc]
	mov	dx,[vtcc.HI_cpFirstTcc]
	cmp	[caT.LO_cpFirstCa],ax
	mov	cx,[caT.HI_cpFirstCa]
	sbb	cx,dx
	jge	LB326

;	     if (vfbSearch.fText || caT.cpFirst == vtcc.cpLim - ccpEop)
;		 {
;LBResetTable:
;		 cpRun = vtcc.cpFirst;
;		 fMatchFoundNonText = fFalse;
;		 goto LBResetSearch;
;		 }
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LB322
	mov	bx,[caT.LO_cpFirstCa]
	mov	cx,[caT.HI_cpFirstCa]
	add	bx,ccpEop
	adc	cx,0
	sub	bx,[vtcc.LO_cpLimTcc]
	sbb	cx,[vtcc.HI_cpLimTcc]
	or	bx,cx
	je	LB322

;	     else
;		 caT.cpFirst = vtcc.cpFirst;
	mov	[caT.LO_cpFirstCa],ax
	mov	[caT.HI_cpFirstCa],dx

;	     }

LB326:
;	 if (caT.cpLim == vtcc.cpLim)
;		 {
;		 if (vfbSearch.fText)
;			 goto LBResetTable;
;		 caT.cpLim -= ccpEop;
;		 }
	mov	bx,[caT.LO_cpLimCa]
	cmp	bx,[vtcc.LO_cpLimCa]
	jne	LB328
	mov	bx,[caT.HI_cpLimCa]
	cmp	bx,[vtcc.HI_cpLimCa]
	jne	LB328
	test	[vfbSearch.fTextFb],maskfTextFb
	jne	LB322
	sub	[caT.LO_cpLimCa],ccpEop
	sbb	[caT.HI_cpLimCa],0

;	 }
LB328:

;    cpMatchStart = caT.cpFirst;
;    vcpMatchLim = caT.cpLim;
;    return cpMatchStart;
	lea	si,[caT]
	errnz	<(cpFirstCa) - 0>
	lodsw
	xchg	ax,bx
	lodsw
	xchg	ax,dx
	errnz	<(cpLimCa) - 4>
	lodsw
	mov	wlo [vcpMatchLim],ax
	lodsw
	mov	whi [vcpMatchLim],ax
	xchg	ax,bx

LB33:
;}
cEnd

LB34:
	mov	ax,si
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	sub	ax,[OFF_cpFirst]
	sbb	dx,[SEG_cpFirst]
	ret

LB345:
	mov	ax,[OFF_cpRun]
	mov	dx,[SEG_cpRun]
LB35:
	;***Begin in-line FetchCpForSearch
;   if (fNotPlain)
;	FetchCpPccpVisible(doc, cpRun, pccpFetch, ww, fFalse /* fNested */);
;   else
;	{
;	FetchCp(doc, cpRun, fcmChars);
;	*pccpFetch = vccpFetch;
;	}
	push	[selCur.docSel]
	push	dx
	push	ax
	errnz	<maskFNotPlainLocal - 080h>
	test	bptr ([wFlags]),maskFNotPlainLocal
	je	LB353
	lea	ax,[ccpFetch]
	push	ax
	push	[selCur.wwSel]
	xor	ax,ax
	errnz	<fFalse>
	push	ax
ifdef DEBUG
	cCall	S_FetchCpPccpVisible,<>
else ;not DEBUG
	cCall	N_FetchCpPccpVisible,<>
endif ;DEBUG
	jmp	short LB357
LB353:
	mov	ax,fcmChars
	push	ax
ifdef DEBUG
	cCall	S_FetchCp,<>
else ;not DEBUG
	cCall	N_FetchCp,<>
endif ;DEBUG
	mov	ax,[vccpFetch]
	mov	[ccpFetch],ax
LB357:
	;***End in-line FetchCpForSearch
	ret

;	LB36 performs:
;	if (icpFetchMac < 128)
;	    rgcpFetch[icpFetchMac++] = vcpFetch;
;	else
;	    {
;	    Assert(icpFetchMac == 128);
;	    blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;	    rgcpFetch[128 - 1] = vcpFetch;
;	    }
;	registers ax, bx, cx are altered
LB36:
	cmp	[icpFetchMac],128
	jl	LB365
ifdef DEBUG
;	/* Assert (icpFetchMac == 128) with a call so
;	as not to mess up short jumps */
	call	LB452
endif ;/* DEBUG */
	push	ds
	pop	es
	push	si
	push	di
	lea	si,[rgcpFetch+4]
	lea	di,[rgcpFetch+0]
	mov	cx,(128-1)*2
	rep	movsw
	pop	di
	pop	si
	dec	[icpFetchMac]
LB365:
	lea	bx,[rgcpFetch]
	mov	ax,[icpFetchMac]
	shl	ax,1
	shl	ax,1
	add	bx,ax
	mov	ax,wlo [vcpFetch]
	mov	[bx],ax
	mov	ax,whi [vcpFetch]
	mov	[bx+2],ax
	inc	[icpFetchMac]
	ret


;    Assert (ichDoc < 0);
ifdef DEBUG
LB37:
	cmp	si,0
	jl	LB38
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,midSearchn
	mov	bx,1010
	cCall	AssertProcForNative,<ax,bx>

	pop	dx
	pop	cx
	pop	bx
	pop	ax
LB38:
	ret
endif ;/* DEBUG */

ifdef DEBUG
LB39:
	push	ax
	push	bx
	push	cx
	push	dx

;    Assert(ccpFetch > 0);
	cmp	[ccpFetch],0
	jg	LB392
	mov	ax,midSearchn
	mov	bx,1011
	cCall	AssertProcForNative,<ax,bx>
LB392:

;    Assert(vcpFetch == cpRun);    /* there can't have been any
;	 invisible chars skipped because we bumped cpLim down past them
;	 already. */
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	sub	ax,[OFF_cpRun]
	sbb	dx,[SEG_cpRun]
	or	ax,dx
	je	LB394
	mov	ax,midSearchn
	mov	bx,1012
	cCall	AssertProcForNative,<ax,bx>
LB394:

;    Assert(icpFetchMac >= 0);
	cmp	[icpFetchMac],0
	jge	LB396
	mov	ax,midSearchn
	mov	bx,1013
	cCall	AssertProcForNative,<ax,bx>
LB396:

;    Debug(CheckRgcpFetch(selCur.ww, &ccpFetch, pbmib->fNotPlain,
;	 &icpFetchMac, rgcpFetch));
	cCall	LN_CheckRgcpFetch

LB40:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;    Assert(vcpFetch + ccpFetch > cpFirst);
ifdef DEBUG
LB41:
	push	ax
	push	bx
	push	cx
	push	dx

	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	add	ax,[ccpFetch]
	adc	dx,0
	mov	bx,[OFF_cpFirst]
	mov	cx,[SEG_cpFirst]
	sub	bx,ax
	sbb	cx,dx
	jl	LB42
	mov	ax,midSearchn
	mov	bx,1014
	cCall	AssertProcForNative,<ax,bx>

LB42:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	     Assert(icpFetchMac == 128);
ifdef DEBUG
LB452:
	push	ax
	push	bx
	push	cx
	push	dx

	cmp	[icpFetchMac],128
	je	LB454
	mov	ax,midSearchn
	mov	bx,1015
	cCall	AssertProcForNative,<ax,bx>

LB454:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	     Assert(ccpRemain > 0);
ifdef DEBUG
LB50:
	push	ax
	push	bx
	push	cx
	push	dx

	or	cx,cx
	jg	LB51
	mov	ax,midSearchn
	mov	bx,1016
	cCall	AssertProcForNative,<ax,bx>

LB51:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	FetchCpPccpVisibleBackward(...)
;-------------------------------------------------------------------------
;/* FetchCpPccpVisibleBackward
;*
;*  Mock mirror image of FetchCpPccpVisible.  Used for searching
;*  backwards, and I can't think of any other uses for it at the
;*  moment.
;*
;*  We want to fetch the last visible run which begins prior to cpRun.
;*
;*  For maximum speed we are going to start looking cbSector characters
;*  before cpRun.  In a long unedited document the run would typically
;*  start here and we will spend little time in this routine.
;*
;*  The other case which might take some time is if we have a lot
;*  of short little runs.  The algorithm is to fetch runs starting
;*  cbSector CP's before the beginning of previously fetched run.
;*  we then sequentially look at successive runs following this point
;*  until we get the last one that begins before the beginning of the
;*  previously fetched run.  In a region with many little runs this
;*  would mean on average many fetches would be performed for each
;*  call to this routine.  To avoid this problem rgcpFetch is kept
;*  around to record beginnings of fetched runs, and this record
;*  is used whenever possible to find the beginning of the desired
;*  run.  This optimization means that in regions of many
;*  little runs we should average about two fetches per call to this
;*  routine.
;*
;*  Care is taken to ensure that rgcpFetch contains only true
;*  beginnings of runs.  That is, if we fetch before CP for any CP
;*  in rgcpFetch, vcpFetch will be either CP or vcpFetch + *pccpFetch
;*  will be less than or equal to CP.
;*
;*  If no visible run is fetched prior to cpLimRun then *pccpFetch is
;*  set to zero before returning.
;*/


;NATIVE FetchCpPccpVisibleBackward(doc, cpLimRun,
;    pccpFetch, ww, fNotPlain, picpFetchMac, rgcpFetch)
;int	doc;
;CP    cpLimRun;
;int	*pccpFetch;
;int	fvc;
;int ww;
;BOOL	 fNotPlain;
;int	*picpFetchMac;
;CP    rgcpFetch[];
;{
;    CP    cpCur;
;    BOOL  fRunStartFetched;
;#ifdef DEBUG
;    int    cBeenHere;
;    CP    cpFetchSav;
;#endif /* DEBUG */


;cProc	 LN_FetchCpPccpVisibleBackward,<PUBLIC,NEAR>,<>
LN_FetchCpPccpVisibleBackward LABEL NEAR

;cBegin
;    Debug(cBeenHere = 0);
ifdef DEBUG
	mov	[cBeenHere],0
endif ;/* DEBUG */

;    if (cpLimRun <= 0)
;	 {
;	 *pccpFetch = 0;
;	 return;
;	 }
	xor	bx,bx
	xor	cx,cx
	sub	bx,ax
	sbb	cx,dx
	jge	LN_FCPVB025
	mov	[OFF_cpLimRun],ax
	mov	[SEG_cpLimRun],dx

;LRestart:
;    Assert(*picpFetchMac >= 0);
LRestart:
ifdef DEBUG
;	/* Assert (*picpFetchMac >= 0) with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB09
endif ;/* DEBUG */

;    if (*picpFetchMac > 0)
;	 /* table of beginning of runs is not empty.
;	    Take one of the runs from the table. */
;	 {
	cmp	[icpFetchMac],0
	jle	LN_FCPVB01

;	 if ((cpCur = rgcpFetch[--(*picpFetchMac)]) >= cpLimRun)
;	     goto LRestart;
	dec	[icpFetchMac]
	mov	ax,[icpFetchMac]
	shl	ax,1
	shl	ax,1
	lea	bx,[rgcpFetch]
	add	bx,ax
	mov	ax,[bx]
	mov	dx,[bx+2]
	mov	cx,ax
	sub	cx,[OFF_cpLimRun]
	mov	cx,dx
	sbb	cx,[SEG_cpLimRun]
	jge	LRestart

;	 (*picpFetchMac)++;
	inc	[icpFetchMac]

;#ifdef DEBUG
;	 FetchCpForSearch(doc, cpCur, pccpFetch,
;		 ww/*fvcScreen*/, fNotPlain);
;	 FetchCpForSearch(doc, vcpFetch + *pccpFetch,
;		 pccpFetch, ww/*fvcScreen*/, fNotPlain);
;	 Assert(*pccpFetch == 0 || vcpFetch >= cpLimRun);
;#endif /* DEBUG */
ifdef DEBUG
;	/* Do this debug stuff with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB11
endif ;/* DEBUG */

;	 FetchCpForSearch(doc, cpCur, pccpFetch,
;		 ww/*fvcScreen*/, fNotPlain);
	call	LB35

;	 Assert(*pccpFetch <= cbSector);
ifdef DEBUG
;	/* Assert (*picpFetchMac <= cbSector) with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB13
endif ;/* DEBUG */

;	 return;
;	 }
	jmp	LN_FCPVB08

LN_FCPVB01:
;    cpCur = cpLimRun;
	mov	ax,[OFF_cpLimRun]
	mov	dx,[SEG_cpLimRun]

;LContinue:
;    while (cpCur > cp0)
;	 {
LN_FCPVB02:
	mov	cx,ax
	or	cx,dx
	jne	Ltemp007
LN_FCPVB025:
	jmp	LN_FCPVB07
Ltemp007:

;	 /* Try looking for a run beginning 1 sector before the
;	    last attempt. */
;	 cpCur = CpMax(cpCur - cbSector, cp0);
	sub	ax,cbSector
	sbb	dx,0
	jge	LN_FCPVB03
	xor	ax,ax
	cwd
LN_FCPVB03:

;	 FetchCpForSearch(doc, cpCur, pccpFetch, ww/*fvcScreen*/, fNotPlain);
	push	dx
	push	ax
	call	LB35
	pop	ax
	pop	dx

;	 Assert(*pccpFetch <= cbSector);
ifdef DEBUG
;	/* Assert (*pccpFetch <= cbSector) with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB15
endif ;/* DEBUG */

;	 if (vcpFetch < cpLimRun)
;	     {
	mov	bx,wlo [vcpFetch]
	mov	cx,whi [vcpFetch]
	sub	bx,[OFF_cpLimRun]
	sbb	cx,[SEG_cpLimRun]
Ltemp021:
	jge	LN_FCPVB02

;	     Assert(cBeenHere++ < 2);
ifdef DEBUG
;	/* Assert (cBeenHere++ < 2) with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB17
endif ;/* DEBUG */

;	     if (vcpFetch + *pccpFetch >= cpLimRun)
;		 return;
	add	bx,[ccpFetch]
	adc	cx,0
	jge	LN_FCPVB08

;	     while (vcpFetch + *pccpFetch < cpLimRun)
;		 {
LN_FCPVB04:
	mov	bx,wlo [vcpFetch]
	mov	cx,whi [vcpFetch]
	add	bx,[ccpFetch]
	adc	cx,0
	sub	bx,[OFF_cpLimRun]
	sbb	cx,[SEG_cpLimRun]
	jge	LN_FCPVB06

;		 if (fRunStartFetched = (vcpFetch > cpCur || cpCur == cp0))
;		     {
;	     /* We know we have the true beginning of a run only if
;		vcpFetch > cpCur or if cpCur == cp0. */

	mov	bx,ax
	or	bx,dx
	je	LN_FCPVB042
	cmp	ax,wlo [vcpFetch]
	mov	bx,dx
	sbb	bx,whi [vcpFetch]
	jge	LN_FCPVB045
LN_FCPVB042:
	db	0B9h	;turns "xor cx,cx" to "mov cx,immediate"
LN_FCPVB045:
	xor	cx,cx
	push	dx
	push	ax	;save cpCur
	jcxz	LN_FCPVB05

;		     Debug(CheckRgcpFetch(ww, pccpFetch, fNotPlain,
;			 picpFetchMac, rgcpFetch));

;		     if (icpFetchMac < 128)
;			 rgcpFetch[(*picpFetchMac)++] = vcpFetch;
;		     else
;			 {
;			 Assert(icpFetchMac == 128);
;			 blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;			 rgcpFetch[128 - 1] = vcpFetch;
;			 }
;	LB36 performs:
;	if (icpFetchMac < 128)
;	    rgcpFetch[icpFetchMac++] = vcpFetch;
;	else
;	    {
;	    Assert(icpFetchMac == 128);
;	    blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;	    rgcpFetch[128 - 1] = vcpFetch;
;	    }
;	registers ax, bx, cx are altered
	call	LB36

;		     }
LN_FCPVB05:

;		 Debug(cpFetchSav = vcpFetch + *pccpFetch);
ifdef DEBUG
;	/* Do this debug line with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB21
endif ;/* DEBUG */
;		 FetchCpForSearch(doc, vcpFetch + *pccpFetch,
;		     pccpFetch, ww/*fvcScreen*/, fNotPlain);
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	call	LB35
	pop	ax
	pop	dx	;restore cpCur

;		 fRunStartFetched = fTrue;
	mov	cx,fTrue

;		 Assert(*pccpFetch <= cbSector);
;		 Assert(vcpFetch >= cpFetchSav);
ifdef DEBUG
;	/* Do these asserts with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB22
endif ;/* DEBUG */

;		 if (*pccpFetch == 0 || vcpFetch >= cpLimRun)
;			 {
;			 if (fRunStartFetched)
;				 goto LRestart;
;			 goto LContinue;
;			 }
;		 }
	cmp	[ccpFetch],0
	je	LN_FCPVB055
	mov	bx,wlo [vcpFetch]
	sub	bx,[OFF_cpLimRun]
	mov	bx,whi [vcpFetch]
	sbb	bx,[SEG_cpLimRun]
	jl	LN_FCPVB04
LN_FCPVB055:
	or	cx,cx
	je	Ltemp021
	jmp	LRestart

LN_FCPVB06:
;	     Debug(CheckRgcpFetch(ww, pccpFetch, fNotPlain,
;		 picpFetchMac, rgcpFetch));
ifdef DEBUG
;	/* Do this debug stuff with a call so as not to mess up
;	short jumps */
	call	LN_FCPVB25
endif ;/* DEBUG */

;	     if (icpFetchMac < 128)
;		 rgcpFetch[(*picpFetchMac)++] = vcpFetch;
;	     else
;		 {
;		 Assert(icpFetchMac == 128);
;		 blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;		 rgcpFetch[128 - 1] = vcpFetch;
;		 }
;	LB36 performs:
;	if (icpFetchMac < 128)
;	    rgcpFetch[icpFetchMac++] = vcpFetch;
;	else
;	    {
;	    Assert(icpFetchMac == 128);
;	    blt(&rgcpFetch[1], &rgcpFetch[0], sizeof(CP)*(128 - 1));
;	    rgcpFetch[128 - 1] = vcpFetch;
;	    }
;	registers ax, bx, cx are altered
	call	LB36

;	     return;
;	     }
;	 }
	jmp	short LN_FCPVB08

LN_FCPVB07:
;    *pccpFetch = 0;
	mov	[ccpFetch],0

LN_FCPVB08:
;}
;cEnd
	ret


;    Assert(*picpFetchMac >= 0);
ifdef DEBUG
LN_FCPVB09:
	push	ax
	push	bx
	push	cx
	push	dx

	cmp	[icpFetchMac],0
	jge	LN_FCPVB10
	mov	ax,midSearchn
	mov	bx,1017
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB10:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
LN_FCPVB11:
	push	ax
	push	bx
	push	cx
	push	dx

;	 FetchCpForSearch(doc, cpCur, pccpFetch, ww/*fvcScreen*/, fNotPlain);
	call	LB35

;	 FetchCpForSearch(doc, vcpFetch + *pccpFetch, pccpFetch, ww/*fvcScreen*/, fNotPlain);
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	call	LB35

;	 Assert(*pccpFetch == 0 || vcpFetch >= cpLimRun);
	cmp	[ccpFetch],0
	je	LN_FCPVB12
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	sub	ax,[OFF_cpLimRun]
	sbb	dx,[SEG_cpLimRun]
	jge	LN_FCPVB12
	mov	ax,midSearchn
	mov	bx,1018
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB12:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	 Assert(*pccpFetch <= cbSector);
ifdef DEBUG
LN_FCPVB13:
	push	ax
	push	bx
	push	cx
	push	dx

	cmp	[ccpFetch],cbSector
	jle	LN_FCPVB14
	mov	ax,midSearchn
	mov	bx,1019
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB14:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	 Assert(*pccpFetch <= cbSector);
ifdef DEBUG
LN_FCPVB15:
	push	ax
	push	bx
	push	cx
	push	dx

	cmp	[ccpFetch],cbSector
	jle	LN_FCPVB16
	mov	ax,midSearchn
	mov	bx,1020
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB16:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

;	     Assert(cBeenHere++ < 2);
ifdef DEBUG
LN_FCPVB17:
	push	ax
	push	bx
	push	cx
	push	dx

	cmp	[cBeenHere],2
	jl	LN_FCPVB18
	mov	ax,midSearchn
	mov	bx,1021
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB18:
	inc	[cBeenHere]
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
LN_FCPVB21:
	push	ax
	push	dx
;		 Debug(cpFetchSav = vcpFetch + *pccpFetch);
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	[OFF_cpFetchSav],ax
	mov	[SEG_cpFetchSav],dx
	pop	dx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
LN_FCPVB22:
	push	ax
	push	bx
	push	cx
	push	dx

;		 Assert(*pccpFetch <= cbSector);
	cmp	[ccpFetch],cbSector
	jle	LN_FCPVB23
	mov	ax,midSearchn
	mov	bx,1023
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB23:

;		 Assert(vcpFetch >= cpFetchSav);
	mov	ax,wlo [vcpFetch]
	mov	dx,whi [vcpFetch]
	sub	ax,[OFF_cpFetchSav]
	sbb	dx,[SEG_cpFetchSav]
	jge	LN_FCPVB24
	mov	ax,midSearchn
	mov	bx,1024
	cCall	AssertProcForNative,<ax,bx>

LN_FCPVB24:
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */

ifdef DEBUG
LN_FCPVB25:
	push	ax
	push	bx
	push	cx
	push	dx

;		     Debug(CheckRgcpFetch(ww, pccpFetch, fNotPlain,
;			 picpFetchMac, rgcpFetch));
	cCall	LN_CheckRgcpFetch

	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret
endif ;/* DEBUG */


;-------------------------------------------------------------------------
;	CheckRgcpFetch(ww, pccpFetch, fNotPlain, picpFetchMac, rgcpFetch)
;-------------------------------------------------------------------------
;#ifdef DEBUG
;NATIVE CheckRgcpFetch(ww, pccpFetch, fNotPlain, picpFetchMac, rgcpFetch)
;int	ww;
;int	*pccpFetch;
;BOOL	 fNotPlain;
;int	*picpFetchMac;
;CP    rgcpFetch[];
;{
;    CP cpFetchSav;
;    int doc = selCur.doc;

ifdef DEBUG
;cProc	 LN_CheckRgcpFetch,<PUBLIC,NEAR>,<>
LN_CheckRgcpFetch LABEL NEAR

;cBegin

;    if (*picpFetchMac > 0)
;	 {
	cmp	icpFetchMac,0
	jle	LN_CRF03

;	 cpFetchSav = vcpFetch;
	mov	ax,wlo [vcpFetch]
	mov	[OFF_cpFetchSav],ax
	mov	ax,whi [vcpFetch]
	mov	[SEG_cpFetchSav],ax

;	FetchCpForSearch(doc, rgcpFetch[*picpFetchMac - 1],
;	     pccpFetch, ww/*fvcScreen*/, fNotPlain);
	lea	bx,[rgcpFetch]
	mov	ax,[icpFetchMac]
	dec	ax
	shl	ax,1
	shl	ax,1
	add	bx,ax
	mov	ax,[bx]
	mov	dx,[bx+2]
	call	LB35

;	 Assert(vcpFetch + *pccpFetch <= cpFetchSav);
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	mov	bx,[OFF_cpFetchSav]
	mov	cx,[SEG_cpFetchSav]
	sub	bx,ax
	sbb	cx,dx
	jge	LN_CRF01
	mov	ax,midSearchn
	mov	bx,1026
	cCall	AssertProcForNative,<ax,bx>
LN_CRF01:

;	 if (vcpFetch + *pccpFetch < cpFetchSav)
;	     {
	or	bx,cx
	je	LN_CRF02

;	     FetchCpForSearch(doc, vcpFetch + *pccpFetch,
;		 pccpFetch, ww/*fvcScreen*/, fNotPlain);
;	     Assert(vcpFetch == cpFetchSav);
	mov	ax,[ccpFetch]
	cwd
	add	ax,wlo [vcpFetch]
	adc	dx,whi [vcpFetch]
	call	LB35

;	     }
LN_CRF02:

;	 FetchCpForSearch(doc, cpFetchSav,
;	     pccpFetch, ww/*fvcScreen*/, fNotPlain);
	mov	ax,[OFF_cpFetchSav]
	mov	dx,[SEG_cpFetchSav]
	call	LB35

;	 }
LN_CRF03:
;}
;cEnd
	ret
;#endif /* DEBUG */
endif ;/* DEBUG */

;-------------------------------------------------------------------------
;	FMatchWhiteSpace(ch)
;-------------------------------------------------------------------------
;NATIVE BOOL FMatchWhiteSpace (ch)
;int	ch;
;{
;    switch (ch)
;	 {
;    case chSpace:
;    case chNonBreakSpace:
;    case chTab:
;	 return fTrue;
;
;    default:
;	 return fFalse;
;	 }
;}

	;LN_FMatchWhiteSpace expects chDoc in dl, and returns equal if true.
	;No registers are changed.
;cProc	 LN_FMatchWhiteSpace,<PUBLIC,NEAR>,<>
LN_FMatchWhiteSpace LABEL NEAR

;cBegin
	cmp	dl,chSpace
	je	LN_FMWS01
	cmp	dl,chNonBreakSpace
	je	LN_FMWS01
	cmp	dl,chTab
LN_FMWS01:
;cEnd
	ret

sEnd    search

        END
