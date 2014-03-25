;   FILEWINN.ASM
;   Core native file routines

; =====================================================================
;   This file contains DOS access routines.
;   These routines are general, simple calls to DOS and
;   are likely to be generally useful.  They assume /PLM calling
;   is used.
;   FAR calls are used throughout
;
;
;   NOTE: DOSLIB.H CONTAINS A C HEADER DEFINING THE FUNCTIONS IN THIS
;   MODULE.  IT MUST BE UPDATED WHENEVER A FUNCTION IS ADDED OR AN INTERFACE
;   CHANGES
; =====================================================================


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
ifdef	PMWORD
	INCL_DOSERRORS	= 1
	include bseerr.inc
	include pmword.inc
endif
        .list

; CODE IN THIS MODULE IS APPENDED TO THE PCODE SEGMENT FOR FILEWIN.C

ifdef TESTCODE	; for PMWORD
createSeg       _TEXT,filewin,byte,public,CODE
else
createSeg       filewin_PCODE,filewin,byte,public,CODE
endif
; DEBUGGING DECLARATIONS

ifdef DEBUG
midFilewinn	equ 20		; module ID, for native asserts
endif

; EXPORTED LABELS

; EXTERNAL FUNCTIONS
ifndef	TESTCODE
externFP	<ReloadSb>
externFP	<ErrorEidProc>
endif

ifdef	PMWORD
externFP	DosClose
externFP	DosRead
externFP	DosWrite
externFP	DosChgFilePtr
externFP	DosGetDateTime
endif

ifdef DEBUG
ifndef TESTCODE
externFP	<AssertProcForNative>
endif
endif

; EXTERNAL DATA
sBegin  data
ifndef TESTCODE
externW vbptbExt	; extern struct BPTB	  vbptbExt;
externW vmerr		; extern struct MERR	  vmerr;
externW vfScratchFile	; extern BOOL		  vfScratchFile;
externW vibpProtect	; extern int		  vibpProtect;
externW mpsbps		; extern SB		  mpsbps[];
endif
sEnd    data

; CODE SEGMENT FILEWIN

sBegin      filewin
        assumes cs,filewin
        assumes ds,dgroup
        assumes ss,dgroup

ifdef ENABLE        ; we use OpenFile instead
;-----------------------------------------------------------------------------
; unsigned FAR WOpenSzFfname( szFfname, openmode )
; PSTR szFfname;  int openmode;
;
; Open specified file in specified mode, return a handle or
; the negative of an error code if the open failed
;-----------------------------------------------------------------------------

; %%Function:WOpenSzFfname %%Owner:peterj
cProc WOpenSzFfname, <FAR, PUBLIC>
parmDP  <szFfname>
parmB   <openmode>
cBegin WOpenSzFfname

    mov     dx,szFfname
    mov     al,openmode
    mov     ah,3dh

    int     21h
    jnc     osfdone
    neg     ax          ; error - return the negative of the error code
osfdone:
cEnd WOpenSzFfname
endif

;-----------------------------------------------------------------------------
; int FAR FCloseDoshnd( doshnd )
;
; Close file given DOS handle, return 0 = error, nonzero = no error
;-----------------------------------------------------------------------------

; %%Function:FCloseDoshnd %%Owner:peterj
cProc FCloseDoshnd, <FAR, PUBLIC>,<si,di>
parmW   <doshnd>
cBegin FCloseDoshnd

ifndef	PMWORD
    mov     bx,doshnd
    mov     ah,3eh
    int     21h
    mov     ax,0000
    jc      cdhskip     ; error, leave a zero in ax
    inc     ax
else	; PMWORD
	cCall	DosClose, <doshnd>
	or	ax, ax
	mov	ax, 0
	jnz	cdhskip
	inc	ax
endif	; PMWORD
cdhskip:
cEnd FCloseDoshnd


;-----------------------------------------------------------------------------
; int CchReadDoshnd ( doshnd, lpchBuffer, bytes )
;
; Read bytes from an open file, place into buffer
; Returns # of bytes read (should be == bytes unless EOF or error)
; If an error occurs, returns the negative of the error code
;-----------------------------------------------------------------------------

; %%Function:CchReadDoshnd %%Owner:peterj
cProc CchReadDoshnd, <FAR, PUBLIC>, <DS,SI,DI>
parmW   <doshnd>
parmD   <lpchBuffer>
parmW   <bytes>
ifdef 	PMWORD
localW	bytesRead
endif	;PMWORD
cBegin CchReadDoshnd

ifndef	PMWORD
    mov     bx,doshnd
    lds     dx,lpchBuffer
    mov     cx,bytes
    mov     ah,3fh

    int     21h
    jnc     crdone
    neg     ax          ; error - return value is the negative of the error code
else	; PMWORD
	lea	ax, bytesRead
	farPtr	lpBytesRead, ds, ax
	cCall	DosRead, <doshnd, SEG_lpchBuffer, OFF_lpchBuffer, bytes, lpBytesRead>
	or	ax, ax
	mov	ax, bytesRead
	jz	crdone
	mov	ax, -ERROR_ACCESS_DENIED	; only error opus understands
endif	; PMWORD	
crdone:
cEnd CchReadDoshnd




;-----------------------------------------------------------------------------
; int CchWriteDoshnd ( doshnd, lpchBuffer, bytes )
;
; Write bytes from an open file, place into buffer
; Returns # of bytes written (should be == bytes unless EOF or error)
; If an error occurs, returns the negative of the error code
; Disk full is not an "error"; detect it by return code != bytes
;-----------------------------------------------------------------------------

; %%Function:CchWriteDoshnd %%Owner:peterj
cProc CchWriteDoshnd, <FAR, PUBLIC>,<DS,SI,DI>
parmW   <doshnd>
parmD   <lpchBuffer>
parmW   <bytes>
ifdef 	PMWORD
localW	bytesWritten
endif	;PMWORD
cBegin CchWriteDoshnd

ifndef PMWORD
    mov     bx,doshnd
    lds     dx,lpchBuffer
    mov     cx,bytes
    mov     ah,40h
    int     21h
    jnc     cwdone
    neg     ax              ; error: return the negative of the error code
else	;PMWORD
	lea	ax, bytesWritten
	farPtr	lpBytesWritten, ds, ax
	cCall	DosWrite, <doshnd, SEG_lpchBuffer, OFF_lpchBuffer, bytes, lpBytesWritten>
	or	ax, ax
	mov	ax, bytesWritten
	jz	cwdone
	mov	ax, -ERROR_ACCESS_DENIED	; only error opus understands
endif	;PMWORD
cwdone:

cEnd CchWriteDoshnd




;-----------------------------------------------------------------------------
; long DwSeekDw ( doshnd, dwSeekpos, bSeekfrom )
;
; Seek to requested position in file
; bSeekfrom is:  0 = seek relative to beginning of file
;                1 = seek relative to current pointer location
;                2 = seek relative to end of file
;
; Returns the new location of the read/write pointer (a long)
; If an error occurs, returns the negative of the error code (long)
;-----------------------------------------------------------------------------

; %%Function:DwSeekDw %%Owner:peterj
cProc DwSeekDw, <FAR, PUBLIC>,<si,di>
parmW   <doshnd>
parmD   <dwSeekpos>
parmB   <bSeekfrom>
ifdef	PMWORD
localD	<dwEndpos>
endif	; PMWORD
cBegin DwSeekDw

ifndef	PMWORD
    mov     bx,doshnd
    mov     cx,SEG_dwSeekpos
    mov     dx,OFF_dwSeekpos
    mov     al,bSeekfrom
    mov     ah,42h

    int     21h
    jnc     seekdone

    neg     ax              ; Error: return the negative of the error code
    mov     dx,0ffffH
else	; PMWORD
	lea	ax, dwEndpos
	farPtr	lpEndpos, ds, ax
	cCall	DosChgFilePtr, <doshnd, SEG_dwSeekpos, OFF_dwSeekpos, bSeekfrom, lpEndpos>
	or	ax, ax
	jz	seekokay
	neg	ax
	mov	dx, 0ffffH
	jmp	short	seekdone
seekokay:
	mov	ax, OFF_dwEndpos
	mov	dx, SEG_dwEndpos
endif	; PMWORD
seekdone:

cEnd DwSeekDw

;-----------------------------------------------------------------------------
; int  FDoshndIsFile( doshnd ) - determines if doshnd is file or device
;
;-----------------------------------------------------------------------------

; %%Function:FDoshndIsFile %%Owner:peterj
cProc FDoshndIsFile, <FAR, PUBLIC>,<si,di>
parmW   <doshnd>
cBegin FDoshndIsFile
	mov	bx,doshnd
	mov	ah,44h  ; IOCTL Data
	mov	al,0	; Get data
	int	21h

	; preset ax to fFalse
	xor	ax,ax
	; carry set on error
	jc	FDIFno
	; bit 7 of dx set for device
	test	dl,80h
	jnz     FDIFno
	; it is a file
	inc	ax
FDIFno:
cEnd FDoshndIsFile

ifndef	PMWORD


ifdef DEBUG
;-----------------------------------------------------------------------------
; void CommSzLib( sz ) - put out string to AUX device
; PSTR sz;
;
; For debugging
;-----------------------------------------------------------------------------

; %%Function:CommSzLib %%Owner:peterj
cProc CommSzLib, <FAR, PUBLIC>,<si,di>
parmDP  <sz>
cBegin CommSzLib

CommSz1:
    mov     bx, sz          ; if ((dl = *(sz++)) == 0)  goto CommSz2
    inc     sz
    mov     dl, [bx]
    or      dl, dl
    jz      CommSz2

    mov     ah,4
    int     21h

    jmp     CommSz1

CommSz2:

cEnd CommSzLib

;-----------------------------------------------------------------------------
; CHAR ChFromComm - get char from AUX device 
;
; For debugging
;-----------------------------------------------------------------------------

; %%Function:ChFromComm %%Owner:peterj
cProc ChFromComm, <FAR, PUBLIC>,<si,di>
cBegin ChFromComm

        mov ah,3
        int 21h
        xor ah,ah

cEnd ChFromComm

endif   ; DEBUG

endif	; PMWORD

ifndef TESTCODE
;-----------------------------------------------------------------------------
; char huge *HpOfBptbExt( ibp )
;-----------------------------------------------------------------------------

PUBLIC HpOfBptbExt
HpOfBptbExt:
    mov     bx,sp
    mov     ax,[bx+4]
    xor     dx,dx
    div     [vbptbExt.cbpChunkBptb]	     ; ax = result, dx = remainder
    add     ax,[vbptbExt.SB_hprgbpExtBptb]
    xchg    ax,dx
    errnz   <cbSector - 512>
    mov     cl,9
    shl     ax,cl
    db	    0CAh, 002h, 000h	;far ret, pop 2 bytes
endif	; TESTCODE
;-----------------------------------------------------------------------------
; void OsTime( pTime )
; struct TIME *ptime;
;
;   pTime is a pointer to a structure of the form:
;       struct {
;               char min;       Minutes (0-59)
;               char hour;      Hours (0-23)
;               char hsec;      Hundredths of seconds (0-99)
;               char sec;       Seconds (0-59)
;              }
;
;   Get current time into structure
;-----------------------------------------------------------------------------

; %%Function:OsTime %%Owner:peterj
cProc OsTime, <FAR, PUBLIC>,<si,di>
parmDP  <pTime>
ifdef PMWORD
localV <dts>, <size DATETIME>
endif
cBegin OsTime

ifndef PMWORD
    mov     ah,2ch
    int     21h

    mov     bx,pTime
    mov     WORD PTR [bx], cx
    mov     WORD PTR [bx+2], dx
else	; PMWORD
	lea	ax, dts
	farPtr	lpdt, ds, ax
	cCall	DosGetDateTime, <lpdt>
	; now move stuff from the os/2 data buffer into the caller's buffer
	mov	ax,ss
	mov	es,ax
	assumes	es,data
	mov	di, pTime	; es:di has place to move to
	mov	al, dts.minutes
	stosb
	mov	al, dts.hours
	stosb
	mov	al, dts.hundreths
	stosb
	mov	al, dts.seconds
	stosb
endif	; PMWORD
cEnd OsTime


;-----------------------------------------------------------------------------
; void OsDate( pDate )
;
;   pDate is a pointer to a structure of the form:
;       struct {
;               int  year;      Year (1980 - 2099)
;               char month;     Month (1-12)
;               char day;       Day (1-31)
;               char dayOfWeek; Week day (0 = Sunday, 6 = Saturday)
;              }
;
;   Get current date into structure
;-----------------------------------------------------------------------------

; %%Function:OsDate %%Owner:peterj
cProc OsDate, <FAR, PUBLIC>,<si,di>
parmDP  <pDate>
ifdef PMWORD
localV <dts>, <size DATETIME>
endif
cBegin OsDate

ifndef PMWORD
    mov     ah,2ah
    int     21h

    mov     bx,pDate
    mov     WORD PTR [bx], cx     ; year
    mov     BYTE PTR [bx+2], dh   ; month
    mov     BYTE PTR [bx+3], dl   ; day
    mov     BYTE PTR [bx+4], al   ; week day
else	; PMWORD
	lea	ax, dts
	farPtr	lpdt, ds, ax
	cCall	DosGetDateTime, <lpdt>
	; now move stuff from the os/2 data buffer into the caller's buffer
	mov	ax,ss
	mov	es,ax
	assumes	es, data
	mov	di, pDate	; es:di has place to move to
	mov	ax, dts.year
	stosw
	mov	al, dts.month
	stosb
	mov	al, dts.day
	stosb
	mov	al, dts.weekday
	stosb
endif	; PMWORD
cEnd OsDate

ifndef TESTCODE
;-------------------------------------------------------------------------
;	IbpLru()
;-------------------------------------------------------------------------
;/* I B P  L R U */
;/* Look for least recently used vbptbExt cache page.
;Returns ibp of page.
;If vfScratchFile and scratch file pages
;routine will strive to maintain a fraction of ext buffers clear.
;This means that if there are too many dirty buffers, a dirty one
;will be picked regardless of lru.
;Note: this version will select a clean page over a dirty page unless 50%
;of the pages are dirty.
;*/
;NATIVE IbpLru()
;{
;	extern struct MERR vmerr;
;	extern BOOL vfScratchFile;
;
;	int ibp, ibpLruClean = 0, ibpLruDirty = 0;
;	int ibpMac = vbptbExt.ibpMac;
;	int cbpsScratchClean = ibpMac;
;	int cbpsClean = 0;
;	int cbpsDirty = 0;
;	int ibpScratch;
;	TS ts, tsLruClean, tsLruDirty;
;	struct BPS HUGE *hpbps = HpbpsIbp(0);
;	TS tsMruBps = vbptbExt.tsMruBps;

cProc	N_IbpLru,<PUBLIC,FAR>,<si,di>

	LocalW	cbpsDirty
	OFFBP_cbpsDirty 	=   -2
	LocalW	ibpLruDirty
	OFFBP_ibpLruDirty	=   -4
	LocalW	tsLruDirty
	OFFBP_tsLruDirty	=   -6
	LocalW	cbpsClean
	OFFBP_cbpsClean 	=   -8
	LocalW	ibpLruClean
	OFFBP_ibpLruClean	=   -10
	LocalW	tsLruClean
	OFFBP_tsLruClean	=   -12
	LocalW	ibpLru
	OFFBP_ibpLru		=   -14
	LocalW	ibpScratch
	OFFBP_ibpScratch	=   -16
ifdef DEBUG
	LocalV	rgchIbpLru,10
endif ;DEBUG

cBegin

;	tsLruClean = tsLruDirty = tsMax;
	;Note: the assembler ts is really 0FFFFh - (the C ts).
	errnz	<tsMax - 0FFFFh>
	push	ds
	pop	es
	xor	ax,ax
	lea	di,[tsLruClean]
	errnz	<OFFBP_ibpLruClean - OFFBP_tsLruClean - 2>
	errnz	<OFFBP_cbpsClean - OFFBP_ibpLruClean - 2>
	errnz	<OFFBP_tsLruDirty - OFFBP_cbpsClean - 2>
	errnz	<OFFBP_ibpLruDirty - OFFBP_tsLruDirty - 2>
	errnz	<OFFBP_cbpsDirty - OFFBP_ibpLruDirty - 2>
	mov	cx,(OFFBP_cbpsDirty - OFFBP_tsLruClean + 2) SHR 1
	rep	stosw

ifdef DEBUG
	push	cx
	push	si
	push	di
	push	ds
	push	es
	push	cs
	pop	ds
	push	ss
	pop	es
	mov	si,offset szIbpLru
	lea	di,[rgchIbpLru]
	mov	cx,cbSzIbpLru
	rep	movsb
	jmp	short IL005
szIbpLru:
	db	'IbpLru',0
cbSzIbpLru equ $ - szIbpLru
IL005:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	cx
endif ;DEBUG
	mov	bx,[vbptbExt.SB_hpmpibpbpsBptb]
	shl	bx,1
	mov	ax,[bx.mpsbps]
	mov	es,ax
	shr	ax,1
	jc	IL007
;	ReloadSb trashes ax, cx, and dx
	cCall	ReloadSb,<>
IL007:
	mov	bx,[vbptbExt.OFF_hpmpibpbpsBptb]
	mov	di,[vbptbExt.ibpMacBptb]

;	Assert(ibpMac >= 5);
ifdef DEBUG
	cmp	di,5
	jge	IL008
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFilewinn
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IL008:
endif ;/* DEBUG */

;	for (ibp = 0; ibp < ibpMac; ibp++, hpbps++)
;		{
	xor	cx,cx	    ;clear out ch so we can use jcxz
	xor	si,si
	jmp	short IL02
IL01:
	inc	si
	add	bx,cbBpsMin
IL02:
	cmp	si,[vbptbExt.ibpMacBptb]
	jae	IL04
	mov	cl,es:[bx.fDirtyBps]

;		if (hpbps->fn == fnScratch)
;			{
	cmp	es:[bx.fnBps],fnScratch
	jne	IL03

;			if (!vfScratchFile)
;				{
;				cbpsScratchClean--;
;				continue;
;				}
	dec	di
	cmp	[vfScratchFile],fFalse
	je	IL01
	inc	di

;			if (hpbps->fDirty)
;				{
;				cbpsScratchClean--;
;				if (ibp != vibpProtect)
;					ibpScratch = ibp;
;				}
;			}
	jcxz	IL03
	dec	di
	cmp	si,[vibpProtect]
	je	IL03
	mov	ax,si
IL03:

;/* normalize time stamps so that the MRU is intMax-1, and the others smaller
;in unsigned arithmetic */
;		ts = hpbps->ts - (tsMruBps + 1);
	;Note: the assembler ts is really 0FFFFh - (the C ts).
	mov	dx,[vbptbExt.tsMruBpsBptb]
	sub	dx,es:[bx.tsBps]

;		if (ibp != vibpProtect)
;			{
	cmp	si,[vibpProtect]
	je	IL01

;/* keep separate tabs on clean and dirty pages so we can choose between them */
;			if (hpbps->fDirty)
;				{
;				cbpsDirty++;
;				if (ts <= tsLruDirty)
;					{
;					tsLruDirty = ts;
;					ibpLruDirty = ibp;
;					}
;				}
;			else
;				{
;				cbpsClean++;
;				if (ts <= tsLruClean)
;					{
;					tsLruClean = ts;
;					ibpLruClean = ibp;
;					}
;				}
;			}
	push	bp
	jcxz	IL035
	add	bp,OFFBP_cbpsDirty - OFFBP_cbpsClean
IL035:
	inc	[cbpsClean]
	cmp	dx,[tsLruClean]
	;Note: Do "jbe" here because the assembler ts is really
	;0FFFFh - (the C ts).
	jb	IL037
	mov	[tsLruClean],dx
	mov	[ibpLruClean],si
IL037:
	pop	bp

;		}
	jmp	short IL01
IL04:

;/* check for no scratch file and buffers almost full (7/8 full) */
;	if (cbpsScratchClean < max(ibpMac >> 3, 4))
;		{
	mov	bx,[vbptbExt.ibpMacBptb]
	shr	bx,1
	shr	bx,1
	shr	bx,1
	cmp	bx,4
	jge	IL055
	mov	bx,4
IL055:
	cmp	di,bx
	jae	IL07

;		if (vfScratchFile)
;			{
;			Assert (ibpScratch != vibpProtect);
;			return ibpScratch;
;			}
	cmp	[vfScratchFile],fFalse
	jne	IL08

;		else
;			{
;			if (!vmerr.fDiskEmerg)
;				{
        mov     cl,[vmerr.fDiskEmergMerr]
        or      [vmerr.fDiskEmergMerr],maskfDiskEmergMerr
        test    cl,maskfDiskEmergMerr
	jnz	IL07

;				ErrorEid(!vmerr.fScratchFileInit ?
;					eidSysLock : eidSysFull,"IbpLru");
;				}
	mov	ax,eidSysLock
	test	[vmerr.fScratchFileInitMerr],maskfScratchFileInitMerr
	je	IL06
	mov	ax,eidSysFull
IL06:
	push	ax
ifdef DEBUG
	lea	ax,[rgchIbpLru]
	push	ax
endif ;DEBUG
	cCall	ErrorEidProc,<>

;			vmerr.fDiskEmerg = fTrue;
;			}
;		}
;	if (tsLruClean > tsLruDirty && cbpsDirty > cbpsClean)
;		/* only return the dirty page if it is older and there are
;		   more dirty pages than clean pages */
;		{
;		Assert (ibpLruDirty != vibpProtect);
;		return ibpLruDirty;
;		}
;	else
;		{
;		Assert (ibpLruClean != vibpProtect);
;		return ibpLruClean;
;		}
IL07:
	mov	ax,[ibpLruClean]
	mov	dx,[tsLruClean]
	cmp	dx,[tsLruDirty]
	;Note: Do "jae" here because the assembler ts is really
	;0FFFFh - (the C ts).
	jae	IL08
	mov	dx,[cbpsDirty]
	cmp	dx,[cbpsClean]
	jbe	IL08
	mov	ax,[ibpLruDirty]
IL08:
ifdef DEBUG
	cmp	ax,[vibpProtect]
	jne	IL09
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midFilewinn
	mov	bx,1002
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
IL09:
endif ;/* DEBUG */

;}
cEnd
endif	;TESTCODE
sEnd        filewin

            END
