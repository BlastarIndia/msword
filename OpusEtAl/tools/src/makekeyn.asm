	.xlist
memC		=	1
?PLM		=	0
?WIN		=	0
	INCLUDE	CMACROS.INC
	.list


;=============================================================================
; CODE SEGMENT
;=============================================================================

sBegin		CODE

	assumes	cs,CODE

;=========================================================================
; CbScanKeyStNat(stKey, pstStr, fSingleString);
;	Scan all strings in array pointed to by pstStr for occurrances
;	of stkey.  Keep a total of the number of bytes saved if all
;	occurrances of stKey are replaced by a one byte token.  If
;	fSingleString is true, only check the one string pointed to
;	by pstStr.
;=========================================================================
cProc	CbScanKeyStNat,<NEAR,PUBLIC>,<ds, si, di>
ParmDP	<stKey, pstStr>
ParmW	<fSingleString>

LocalW	<cbSave, cbScan>

cBegin
	mov	cbSave,0		;  cbSave = 0;
	les	di,stKey		;  if (stKey == NULL)
	and	di,di			;
	jz	CSKret			;    return(0);
	mov	ah,es:[di+1]		;  chFirst = *(stKey+1);
	mov	cl,es:[di]		;  cbKey = *stKey - 1;
	xor	ch,ch			;
	dec	cx			;  if (cbKey == 0)
	jz	CSKret			;    return(0);
	mov	dx,cx			;
While1:	lds	bx,pstStr		;  while ((pchScan = *pstStr++) != NULL)
	add	OFF_pstStr,4		;
	lds	bx,ds:[bx]		;
	and	bx,bx			;
	jz	EndWhile1		;    {
	mov	cl,ds:[bx]		;    cbScan = (int)*pchScan++;
	inc	bx			;
	xor	ch,ch			;
	mov	cbScan,cx		;
ForLoop:				;    for (;;)
					;      {
	mov	si,ds			;
	mov	es,si			;
	mov	di,bx			;      while (cbScan-- > 0)
	mov	cx,cbScan		;        if (*pchScan++ == chFirst)
	mov	al,ah			;          break;
	repne	scasb			;
	cmp	cx,dx			;      if (cbScan < cbKey)
	jl	EndFor			;        break;
	mov	bx,di			;
	mov	cbScan,cx		;
	mov	cx,dx			;      cb = cbKey;
	mov	si,bx			;      pch = pchScan;
	les	di,stKey		;      pchKey = stKey+2;
	add	di,2			;
	repe	cmpsb			;      while (cb-- > 0)
					;        if (*pchKey++ != *pch++)
					;          break;
	jne	ForLoop			;      if (cb == -1)
					;        {
	add	cbSave,dx		;        cbSave += cbKey;
	mov	bx,si			;        pchScan = pch - 1;
	dec	bx			;
	sub	cbScan,dx		;        cbScan -= cbKey;
					;        }
	jmp	ForLoop			;      }
EndFor:					;
	test	fSingleString,0ffffh	;    if (fSingleString)
	jz	While1			;      break;
					;    }
EndWhile1:				;
CSKret:					;
	mov	ax,cbSave		;  return(cbSave);
cEnd

sEnd	CODE

	end

