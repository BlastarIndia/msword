;   FILE2N.ASM
;   Non-core file routines
;


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

; CODE IN THIS MODULE IS APPENDED TO THE PCODE SEGMENT FOR FILE2.C
ifdef	PMWORD
externFP	DosQFileMode
externFP	DosQCurDir
externFP	DosQCurDisk
externFP	DosSelectDisk
externFP	DosChDir
externFP	DosMkDir
externFP	DosRmDir
externFP	DosDelete
externFP	DosMove
externFP  	DosFindFirst
externFP  	DosFindNext
externFP  	DosFindClose
externFP	DosExecPgm
externFP	DosQFSInfo
externFP	DosQHandType
endif

ifdef TESTCODE	; for PMWORD!
createSeg       _TEXT,file2,byte,public,CODE
else
createSeg       file2_PCODE,file2,byte,public,CODE
endif

ifndef TESTCODE	; For PMWORD
EXTRN   ANSITOOEM:FAR
externFP	GetCurrentPDB
endif


sBegin  data

ifndef TESTCODE
externW                 vbptbExt;
endif

ifdef PMWORD
staticW	hDirFde,0	; Handle used by FindFirst/FindNext/FindClose
staticB	ffLatch,0			; Set to 1 during FindFiles.
endif ; PMWORD

sEnd    data

sBegin      file2
        assumes cs,file2
        assumes ds,dgroup
        assumes ss,dgroup

;-----------------------------------------------------------------------------
; WORD DaGetFileModeSz(sz)
; PSTR sz;
;
; Return a word indicating attributes of file sz.  Return 0xFFFF if it fails.
;-----------------------------------------------------------------------------
; %%Function:DaGetFileModeSz %%Owner:peterj

cProc DaGetFileModeSz, <FAR, PUBLIC>,<si,di>
parmDP  <sz>
ifndef PMWORD
cBegin DaGetFileModeSz
    mov     dx,sz
    mov     ax,4300h
    int     21h
    mov     ax, cx
    jnc     daNoErr
    mov     ax, 0ffffh      ; error -- return 0xFFFF
daNoErr:
else	;PMWORD
localW	result
cBegin DaGetFileModeSz
	xor		ax, ax
	lea		bx, result
	farPtr		lpsz, ss, sz
	farPtr		lpresult, ss, bx
	cCall		DosQFileMode, <lpsz, lpresult, ax, ax>
	or		ax, ax
	mov		ax, result	; assume no error
	jz		daNoErr
	mov		ax, 0ffffh      ; error -- return 0xFFFF
daNoErr:
endif	; PMWORD
cEnd DaGetFileModeSz

; ---------------------------------------------------------------------
; int FAR CchCurSzPathNat( szPath, bDrive )
; PSTR szPath;
; int bDrive;
;
; Copy the current path name for the current drive into szPath
; szPath must have 67 bytes of storage
;
; bDrive is 0=default, 1=A, 2=B,...
;
; Returned cch includes the terminating '\0'
; Form of returned string is e.g. "C:\WINDOWS\BIN\" (0-terminated)
; String is guaranteed to: (1) Include the drive letter, colon, and leading "\"
;                          (2) End with a backslash
;
; 0 = an error occurred, nonzero = success
; the path string will be NULL if an error occurred
; An error should really not be possible, since the default drive ought to be
; valid
; ---------------------------------------------------------------------
; %%Function:CchCurSzPathNat %%Owner:peterj
cProc       	CchCurSzPathNat, <FAR, PUBLIC>, <SI,DI>
parmDP      	<szPath>
parmB       	<bDrive>
ifdef 	PMWORD
localD		drivemap
localW		drive
localW		buflength
endif	;PMWORD
cBegin      	CchCurSzPathNat
		mov     al,bDrive
		mov     dl,al
		cmp     al,0
		jz      cspDFLTDRV  ; default drive
		dec     al
		jmp     cspGOTDRV   ; not default drive

cspDFLTDRV:
ifndef 	PMWORD
	        mov     ah,19h      ; Get current drive
            	int     21h
else	;PMWORD
		lea	bx, drivemap
		farPtr	lpdrivemap, ss, bx
		lea	ax, drive
		farPtr	lpdrive, ss, ax
		cCall	DosQCurDisk, <lpdrive, lpdrivemap>
		or	ax, ax
		jnz	cspERR
		mov	ax, drive
		dec	ax	; OS/2 returns A=1, B=2, rather than
				; A=0, B=1, as under DOS
endif	;PMWORD
            	; now we have al: 0=A, 1=B, ....
            	; and under DOS  dl: 0=default, 1=A, 2=B

cspGOTDRV:                      ; Put "X:\" at front of szPath
		add     al,'A'
		mov     si,szPath
		mov     [si],al
		mov     BYTE PTR [si+1],':'
		mov     BYTE PTR [si+2],'\'  ; Leave si pointing at szPath
		add     si,3        ; Rest of path goes at SzPath+3
ifndef	PMWORD
		mov     ah,47h
		int     21h
		mov     si,szPath
		jc      cspERR      ; error -- return negative of error code in ax
else 	; PMWORD
		mov	buflength, 64		; max length
		lea	ax, buflength
		farPtr	lplength, ss, ax
		farPtr	lpszPath, ss, si
		cCall	DosQCurDir, <bDrive, lpszPath, lplength> 
		or	ax, ax
		jz	cspNoErr
		xor	ax, ax
		jmp	cspRET
cspNoErr:
		mov     si,szPath
	
endif	; PMWORD
		dec     si          ; Path was OK - find null terminator
cspLOOP:    	inc     si
		cmp     al,[si]
		jnz     cspLOOP

		cmp     BYTE PTR [si-1],'\' ; Append backslash if needed
		jz      cspSTROK            ; not needed, string is already OK
		mov     BYTE PTR [si],'\'
		inc     si
		mov     BYTE PTR [si],0
cspSTROK:                               ; now we are guaranteed a good string
		mov     ax,si               ; determine string length
		sub     ax,szPath
		inc     ax
		jmp     short cspRET

cspERR:     	mov     BYTE PTR [si],0         ;  error -- NULL path string
		neg     ax
cspRET:
cEnd        	CchCurSzPathNat


;---------------------------------------------------------------
; int FAR FSetCurStzPath( stzPath )
; CHAR * stzPath;
;
;  This routine sets the current directory.
;
;  stzPath is a path specification in the form
;   
;            C:\...\...  (drive letter is optional)
;
; Returns:  0 if failure
;           non-zero if success
;
; %%Function:FSetCurStzPath %%Owner:peterj
cProc	FSetCurStzPath,<PUBLIC,FAR>,<si,di>
ParmW	stzPath

cBegin  FSetCurStzPath
	mov	si,stzPath		; ds:si = stzPath
	cmp	byte ptr [si],0 	; anything to do?
        mov     ax,1
	jz	scspRet                 ; no, done

	cmp	byte ptr [si+2],":"	; was a drive specified?
	jnz	scspSetPath		; no, set path

	mov	dl,[si+1]		; get drive letter requested
	or	dl,20h			; make it lower case
ifndef PMWORD
	sub	dl,"a"			; bias it so A = 0, B = 1, etc.
	mov	ah,0eH			; set current disk
	int	21H			; 
	mov	ah,19H			; now get current drive
	int	21H
	cmp	dl,al			; check if current=requested
	jz	scspCheckForMore	; yes, now look at rest of path
	mov	ax,0			; no, indicate error
	jmp	scspRet			;     and return
else	;PMWORD
	sub	dl, "`"			; bias is so a = 1, b= 2, etc
	xor	dh, dh
	cCall	DosSelectDisk, <dx>
	or	ax, ax			; Error
	jz	scspCheckForMore
	xor	ax, ax			; Yes, set to 0
	jmp	scspRet
endif	;PMWORD
scspCheckForMore:
	cmp	byte ptr [si],2 	; it that all there is?
        mov     ax,1
	jz	scspRet			; yes, all done

scspSetPath:
	mov	dx,si
	inc	dx			; ds:dx points to string
ifndef	PMWORD
	mov	ah,3bh			; set path
	int	21h
	cmc                             ; carry bit now set if success
        sbb     ax,ax                   ; if carry, AX = -1, else AX = 0
else	;PMWORD
	farPtr	lpDir, ds, dx
	xor 	ax, ax
	cCall DosChDir, <lpDir, ax, ax>
	or	ax, ax
	jz	scspOkay
	mov	ax, 0ffffh
scspOkay:
	inc	ax			;	Non zero
endif	;PMWORD
scspRet:
cEnd    FSetCurStzPath



;---------------------------------------------------------------
; int FAR FMakeStzPath( stzPath )
; CHAR * stzPath;
;
;  This routine makes a new directory.
;
;  stzPath is a path specification in the form
;   
;            C:\...\...  (drive letter is optional)
;
; Returns:  negative of dos error code if failure
;           0 if null path
;           non-zero if success
;
; Added (6/29/88, Tony Krueger) by modifying existing FSetCurStzPath.
; %%Function:FMakeStzPath %%Owner:peterj
cProc	FMakeStzPath,<PUBLIC,FAR>,<si,di>
ParmW	stzPath

cBegin  FMakeStzPath
	mov	si,stzPath		; ds:si = stzPath
	cmp	byte ptr [si],0 	; anything to do?
        mov     ax,0
	jz	mspRet                  ; no, return error (0 in ax)

mspSetPath:
	mov	dx,si
	inc	dx			; ds:dx points to string
ifndef PMWORD
	mov	ah,39h			; function 39h: make directory
	int	21h
	jnc	mspRet
	neg	ax			; error - return neg. of error code
else	;PMWORD
	farPtr	lpDir, ds, dx
	xor 	ax, ax
	cCall	DosMkDir, <lpDir, ax, ax>
	or	ax, ax
	jz	mspOkay

	; Error, translate into DOS err codes. Path Not Found and Access Denied are
	; the same as DOS, but OS/2 can also return Drive Locked and Not DOS Disk, so
	; map these to Access Denied.

	cmp	ax, ERROR_PATH_NOT_FOUND
	mov	ax, -3			; negative of dos error for path not found
	jz	mspRet
	mov	ax, -5			; negative of dos error for access denied
	jmp	short	mspRet
mspOkay:
	inc	ax			;	Non zero
	
endif	;PMWORD
mspRet:
cEnd    FMakeStzPath



;---------------------------------------------------------------
; int FAR FRemoveStzPath( stzPath )
; CHAR * stzPath;
;
;  This routine removes (deletes) a directory.
;
;  stzPath is a path specification in the form
;   
;            C:\...\...  (drive letter is optional)
;
; Returns:  negative of dos error code if failure
;           0 if null path
;           non-zero if success
;
; Added (6/29/88, Tony Krueger) by modifying existing FSetCurStzPath.
; %%Function:FRemoveStzPath %%Owner:peterj
cProc	FRemoveStzPath,<PUBLIC,FAR>,<si,di>
ParmW	stzPath

cBegin  FRemoveStzPath
	mov	si,stzPath		; ds:si = stzPath
	cmp	byte ptr [si],0 	; anything to do?
        mov     ax,0
	jz	dspRet                  ; no, return error (0 in ax)

dspSetPath:
	mov	dx,si
	inc	dx			; ds:dx points to string
ifndef	PMWORD
	mov	ah,3ah			; function 3Ah: remove directory
	int	21h
	jnc	dspRet
	neg	ax			; error - return neg. of error code
else	;PMWORD
	farPtr	lpDir, ds, dx
	xor 	ax, ax
	cCall	DosRmDir, <lpDir, ax, ax>
	or	ax, ax
	jz	dspOkay

	; Error, translate into DOS err codes. Path Not Found , Access Denied 
	; Current Directory and Dir Not Empty are the same as DOS, 
	; but OS/2 can also return Drive Locked, Not DOS Disk, and
	; File Not Found, so map these to Access Denied.

	cmp	ax, ERROR_DRIVE_LOCKED
	jz	dspSetToDenied
	cmp	ax, ERROR_NOT_DOS_DISK
	jz	dspSetToDenied
	cmp	ax, ERROR_FILE_NOT_FOUND
	jz	dspSetToDenied
	neg	ax
	jmp	short 	dspRet
dspSetToDenied:
	mov	ax, -5			; negative of dos error for access denied
	jmp	short	dspRet
dspOkay:
	inc	ax			;	Non zero
	
endif	;PMWORD

dspRet:
cEnd    FRemoveStzPath


ifdef ENABLE  ; use OpenFile under windows
;-----------------------------------------------------------------------------
; unsigned FAR WCreateNewSzFfname( szFfname, attrib )
; PSTR szFfname;  int attrib;
;
; Create specified file, leave open for read/write, return handle
; filename is an ffname, with drive and path. Uses the NEW
; DOS 3.0 CREATE call which fails if the file exists. Caller has
; responsibility for assuring DOS version number sufficiently high
;
; returned handle is negative if there was an error
; the value will be the negative of the error code returned in AX
;-----------------------------------------------------------------------------
; %%Function:WCreateNewSzFfname %%Owner:peterj

cProc WCreateNewSzFfname, <FAR, PUBLIC>,<si,di>
parmDP  <szFfname>
parmW   <attrib>
cBegin WCreateNewSzFfname

    mov     dx,szFfname
    mov     cx,attrib
    mov     ah,5bh
    int     21h
    jnc     cnsfdone
    neg     ax          ; error - return the negative of the error code
cnsfdone:
cEnd WCreateNewSzFfname

;-----------------------------------------------------------------------------
; unsigned FAR WCreateSzFfname( szFfname, attrib )
; PSTR szFfname;  int attrib;
;
; Create specified file, leave open for read/write, return handle
; filename is an ffname, with drive and path
;
; returned handle is negative if there was an error
; the value will be the negative of the error code returned in AX
;-----------------------------------------------------------------------------
; %%Function:WCreateSzFfname %%Owner:peterj

cProc WCreateSzFfname, <FAR, PUBLIC>,<si,di>
parmDP  <szFfname>
parmW   <attrib>
cBegin WCreateSzFfname

    mov     dx,szFfname
    mov     cx,attrib
    mov     ah,3ch
    int     21h
    jnc     csfdone
    neg     ax          ; error - return the negative of the error code
csfdone:
cEnd WCreateSzFfname
endif ; ENABLE

ifndef PMWORD
;-----------------------------------------------------------------------------
; int DosxError()
;
; Return a DOS extended error code
;-----------------------------------------------------------------------------

; %%Function:DosxError %%Owner:peterj
cProc DosxError, <FAR, PUBLIC>,<si,di>
cBegin DosxError
    mov     ah,59h
    mov     bx,0
    int     21h
cEnd DosxError
endif	;PMWORD

;-----------------------------------------------------------------------------
; int FAR EcDeleteSzFfname( szFfname )
; PSTR szFfname;
;
; Delete specified file, return < 0=failure, 0=success
;-----------------------------------------------------------------------------

; %%Function:EcDeleteSzFfname %%Owner:peterj
cProc EcDeleteSzFfname, <FAR, PUBLIC>,<si,di>
parmDP  <szFfname>

localV  <szOem>,cchMaxFile

cBegin EcDeleteSzFfname

    ; Convert filename from ANSI set to OEM Set

ifndef TESTCODE
    push    ds
    push    szFfname
    push    ds
    lea     ax,szOem
    push    ax
    call    ANSITOOEM
    lea     dx,szOem
else	; TESTCODE
    mov	    dx, szFfname
endif	; TESTCODE
ifndef PMWORD
    mov     ah,41h

    int     21h
    jc      dsfskip     ; error - return the negative of the error code
    mov     ax,0ffffh
dsfskip:
    neg     ax
else	; PMWORD
	farPtr	lpDir, ds, dx
	xor 	ax, ax
	cCall	DosDelete, <lpDir, ax, ax>
	or	ax, ax
	jz	dsfOkay
	cmp	ax, ERROR_FILE_NOT_FOUND
	je	dsfNegate
	mov	ax, ERROR_ACCESS_DENIED	; map all other errors to access_denied
dsfNegate:
	neg	ax
dsfOkay:
endif	; PMWORD
cEnd EcDeleteSzFfname

;-----------------------------------------------------------------------------
; int FAR EcRenameSzFfname( szCur, szNew )
; PSTR szCur, szNew;
;
; Rename file szCur to szNew, return < 0=failure, 0=success
;-----------------------------------------------------------------------------

; %%Function:EcRenameSzFfname %%Owner:peterj
cProc EcRenameSzFfname, <FAR, PUBLIC>, <ES,SI,DI>
parmDP  <szCur>
parmDP  <szNew>

localV  <szCurOem>,cchMaxFile
localV  <szNewOem>,cchMaxFile

cBegin EcRenameSzFfname

    ; Convert filenames to Oem char set
ifndef TESTCODE
    push    ds
    push    szCur
    push    ds
    lea     ax,szCurOem
    push    ax
    call    ANSITOOEM
    push    ds
    push    szNew
    push    ds
    lea     ax, szNewOem
    push    ax
    call    ANSITOOEM
endif	; TESTCODE
ifndef PMWORD
    lea     dx,szCurOem   ; old filename in ds:dx
    push    ds            ; new filename in es:di
    pop     es
    lea     di,szNewOem
    mov     ah,56h

    int     21h
    jc      rnfskip     ; error - return the negative of the error code
    mov     ax,0ffffh
rnfskip:
    neg     ax
else	; PMWORD
ifdef TESTCODE
	mov	dx, szCur
	mov	di, szNew
else	; TESTCODE
	lea	dx, szCurOem
	lea	di, szNewOem
endif	; TESTCODE

	farPtr	lpOld, ds, dx
	farPtr	lpNew, ds, di
	xor	ax, ax
	cCall	DosMove, <lpOld, lpNew, ax, ax>
	or	ax, ax
	jz	rnfOkay
	cmp	ax, ERROR_FILE_NOT_FOUND
	je	rnfNegate
	cmp	ax, ERROR_PATH_NOT_FOUND
	je	rnfNegate
	cmp	ax, ERROR_NOT_SAME_DEVICE
	je	rnfNegate
	mov	ax, ERROR_ACCESS_DENIED	; map all other errors to access_denied
rnfNegate:
	neg	ax
rnfOkay:
endif 	; PMWORD
cEnd EcRenameSzFfname

ifndef PMWORD

;-----------------------------------------------------------------------------
; int FAR FFirst(pb, szFileSpec, attrib)
; BYTE NEAR *pb;
; PSTR szFileSpec; int attrib;
;
; Get first directory entry, place in buffer at pb. (buffer must contain
;                                                    43 bytes of storage)
; attrib specifies attribute per MSDOS spec.
; szFileSpec is filename specification
; Returns 0=no error, nonzero = error
;-----------------------------------------------------------------------------

; %%Function:FFirst %%Owner:peterj
cProc FFirst, <FAR, PUBLIC>, <SI, DI>
parmDP  <pb, szFileSpec>
parmW   <attrib>
cBegin FFirst

    mov     dx,pb       ; set dta to pb
    mov     ah,1ah
    int     21h

    mov     cx,attrib   ; get first directory record, place in *pb
    mov     dx,szFileSpec
    mov     ah,4eh
    int     21h
    jc     ffdone
    xor     ax,ax

ffdone:
cEnd FFirst


;-----------------------------------------------------------------------------
; int FAR FNext(pb)
; BYTE NEAR *pb;
;
; Get next directory entry, place in buffer at pb.
; Return 0= found match OK, nonzero = error or no more matches
;-----------------------------------------------------------------------------

; %%Function:FNext %%Owner:peterj
cProc FNext, <FAR, PUBLIC>, <SI, DI>
parmDP  <pb>
cBegin FNext

    mov     dx,pb       ; set dta to pb
    mov     ah,1ah
    int     21h

    mov     ah,4fh
    int     21h
    jc     fndone
    xor     ax,ax

fndone:
cEnd FNext

else	; PMWORD


;********** FindClose *********
;*	entry : none.
;*	* close directory handle
;*	exit : n/a
;*
;*  It seems that DosFindClose will return an error if you've already
;*  found all the files with DosFindNext.

cProc	FindClose,<FAR,PUBLIC>
cBegin	FindClose


	push	hDirFde
	call	DosFindClose
	mov	[ffLatch], 0		; We're done - lift the latch.


cEnd	FindClose

;-----------------------------------------------------------------------------
; int FAR FFirst(pfde, szPath, attrib)
; BYTE NEAR *pfde;
; PSTR szPath; 
; int attrib;
;
; Get first directory entry, place in buffer at pfde. (buffer must contain
;                                                    43 bytes of storage)
; attrib specifies attribute per MSDOS spec.
; szPath is filename specification
; Returns 0=no error, nonzero = error
;-----------------------------------------------------------------------------

cProc FFirst, <FAR, PUBLIC>, <SI, DI>
parmDP  <pfde, szPath>
parmW   <atr>
localW centryFirst			; Number of directory entries.
localV FindFirstBuf,<size FDE_OS2>	; Xfer from here to passed-in buffer
cBegin	FFirst

; 	OS/2 requires a FindClose at the end of a search, but DOS doesn't.
;	Therefore, drop a latch before the FindFirst, and each successive
;	time through FIndFirst, check it. If the latch is set, do a FindClose
;	before doing the FindFirst.
	
	xor	bx, bx
	cmp	[ffLatch],bh		; Is the latch dropped? (bh=0)
	je	FFNoClose		; Jump if it's not.

; The latch is dropped.  That means the app didn't do a Find Close after its
; last set of FindFirst/FindNexts.  Clean up for her, and continue.

	call	FindClose

FFNoClose:
	mov	[ffLatch],1		; Drop the latch.
	PushArg	<ss, szPath>		;* Path file name

	mov	[hDirFde],-1		;* request new directory handle
	PushArg	<ds>			;* place for
	PushArg	<offset hDirFde>	;*   directory handle

	PushArg	<atr>			;* attribute for match

	lea	ax,FindFirstBuf
	PushArg	<ss, ax>		;* result buffer
	PushArg	<size FDE_OS2>		;* size of result buffer

	lea	bx,centryFirst
	mov	wo [bx],1
	PushArg	<ss, bx>

	xor	ax,ax
	PushArg	<ax>
	PushArg	<ax>

	cCall	DosFindFirst
	or	ax,ax
	mov	ax,-1			; Preserve flags!
	jnz	FF99			; Jump if none found.
;	AssertEq	centryFirst,1	; Should have found exactly 1 file.

;   Transfer info from FindFirstBuf.FDE_OS2 structure to pfde.FDE struct.

	mov	ax,ss
	mov	es,ax
	assumes	es,data
	mov	bx,pfde
	lea	di,[bx.atrFde]		; Where we transfer to.

	mov	ax,FindFirstBuf.attrFileFdeOS2
	xor	ah,ah
	stosb

	mov	ax,FindFirstBuf.wTimeWriteFdeOS2
	stosw
	
	mov	ax,FindFirstBuf.wDateWriteFdeOS2
	stosw
	
	mov	ax,FindFirstBuf.OFF_cbFileFdeOS2
	stosw

	mov	ax,FindFirstBuf.SEG_cbFileFdeOS2
	stosw

	xor	cx,cx
	mov	cl,FindFirstBuf.cchNameFdeOS2
	lea	si,FindFirstBuf.szNameFdeOS2	; Where we transfer from.
	rep	movsb				; Transfer the filename.
	xor	ax,ax
	stosb					; Zero-terminate.

	xor	ax, ax				; Success
FF99:
cEnd	FFirst


;-----------------------------------------------------------------------------
; int FAR FNext(pfde)
; BYTE NEAR *pfde;
;
; Get next directory entry, place in buffer at pfde.
; Return 0= found match OK, nonzero = error or no more matches
;-----------------------------------------------------------------------------

cProc FNext, <FAR, PUBLIC>, <SI, DI>
parmDP pfde
localW centryNext			; Number of directory entries.
localV FindNextBuf,<size FDE_OS2>	; Xfer from here to passed-in buffer
cBegin FNext


	PushArg	<hDirFde>	;* directory handle
	
	lea	ax,FindNextBuf
	PushArg	<ss, ax>		;* result buffer
	PushArg	<size FDE_OS2>		;* size of result buffer

	lea	bx,centryNext
	mov	wo [bx],1
	PushArg	<ss, bx>

	cCall	DosFindNext
	or	ax,ax
	mov	ax,-1			; Preserve flags!
	jnz	FN99			; Jump if none found.
;	AssertEq	centryFirst,1	; Should have found exactly 1 file.

;   Transfer info from FindNextBuf.FDE_OS2 structure to pfde.FDE struct.

	mov	ax,ss
	mov	es,ax
	assumes	es,data
	mov	bx,pfde
	lea	di,[bx.atrFde]		; Where we transfer to.

	mov	ax,FindNextBuf.attrFileFdeOS2
	xor	ah,ah
	stosb

	mov	ax,FindNextBuf.wTimeWriteFdeOS2
	stosw
	
	mov	ax,FindNextBuf.wDateWriteFdeOS2
	stosw
	
	mov	ax,FindNextBuf.OFF_cbFileFdeOS2
	stosw

	mov	ax,FindNextBuf.SEG_cbFileFdeOS2
	stosw

	xor	cx,cx
	mov	cl,FindNextBuf.cchNameFdeOS2
	lea	si,FindNextBuf.szNameFdeOS2	; Where we transfer from.
	rep	movsb				; Transfer the filename.
	xor	ax,ax
	stosb					; Zero-terminate.

	xor	ax, ax				; Success
FN99:

cEnd	FNext

endif	; PMWORD

ifndef	PMWORD

;-----------------------------------------------------------
;
; ShellExec (file, args) - Exec or load program.
;
;-----------------------------------------------------------
;
; %%Function:ShellExec %%Owner:peterj
cProc	ShellExec,<PUBLIC,FAR>,<si,di>
    parmW file
    parmW args			;

cBegin
    mov     dx,file
    push    ds
    pop     es
    mov     bx,args
    mov     ax,4b00h
    int     21h
cEnd
else	;PMWORD
;---------------------------------------------------------------------------
; The following routine is a modified version of RerrExec from the CW code.
; It was taken from \\applib\slmro\lib\cw\kernel\krun5.asm 24-7-1989, and
; modified to fit the OPUS ShellExec API	- ricks
; REVIEW: Work on this halted until further info on SW_ problem
;----------------------------------------------------------------------------
endif ;PMWORD

;-----------------------------------------------------------------------------
; unsigned long FAR LcbDiskFreeSpace( chDrive )
;
; Return the number of bytes of free space available on the specified drive.
; chDrive == { 'A', 'B',... } | 0 (for default drive)
;
; If the cluster size exceeds 64K, the granularity of the result will reflect the
; dropping of a number of bits sufficient to fit the cluster size in a word.
;
; If an error code occurs, returns -1L.
;
;-----------------------------------------------------------------------------

; %%Function:LcbDiskFreeSpace %%Owner:peterj
cProc LcbDiskFreeSpace, <FAR, PUBLIC>,<si,di>
parmW   <chDrive>
ifdef	PMWORD
localV	pbQfs,<size qfs>
endif	; PMWORD
cBegin LcbDiskFreeSpace

    mov     dx, chDrive
    or      dx,dx               ; default drive (letter == 0), do not adjust
    jz      LDefault
    sub     dl, ('A' - 1)       ; convert drive letter {A,B,..} to {1,2,..}
LDefault:
ifndef PMWORD
    mov     ah,36h              ; GetDiskFreeSpace
    int     21h
    jnc     LOk
    cwd                         ; Error - Leave FFFF:FFFF in AX:DX
    jmp     LDone
LOk:                            
else ; PMWORD
	; at this point dl has drive index, or 0 for default
	xor	dh, dh
	mov	ax, 1
	mov	bx, size qfs
	lea	cx, pbQfs
	farPtr	lpQfs, ds, cx
	cCall	DosQFSInfo, <dx, ax, lpQfs, bx>
	or	ax, ax
	jz	LOk
	mov	ax, 0FFFFh
	cwd
	jmp	LDone
LOk:
	mov	ax,wo pbQfs.cSectorUnit	; sectors/cluster
	mov	bx,wo pbQfs.cUnitAvail	; free clusters
	mov	cx,wo pbQfs.cbSect	; bytes/sector
	mov	dx,wo pbQfs.cUnit	; clusters/drive
endif ; PMWORD
				; got size OK:  bx - clusters avail
                                ;               dx - clusters total (ignore)
                                ;               cx - bytes per sector
                                ;               ax - sectors per cluster
                                ;
                                ; our return value is
                                ;  (long) bx * (long) cx * (long)ax
    mul     cx
    xor     cx, cx              ; zero out adjustment amount
LTestDx:
    or      dx,dx
    jz      LShort
                                ; this means cluster size exceeds 64K
    shr     dx,1                ; adjust until cluster size < 64K
    ror     ax,1
    inc     cx
    jmp     LTestDx

LShort:
    mul     bx
    jcxz    LDone               ; cx = 0 means no adjustment is necessary
                                ; (cluster size < 64K)
LAdjust:
    sal     ax,1                ; adjust dx:ax by the amount we compensated
    rol     dx,1                ; above
    loop    LAdjust

LDone:

cEnd LcbDiskFreeSpace

ifdef NOTUSED
;-----------------------------------------------------------------------------
;
;   long TsGetLastUpdateDosh(dosh)
;       Given a dos-handle, it will give back its time stamp when it was
;       last updated.  It is formatted so that if ts1 < ts2, then the file
;       associated with dosh1 is updated before dosh2.  The resolution is
;       2 seconds.
;
;-----------------------------------------------------------------------------
; %%Function:TsGetLastUpdateDosh %%Owner:NOTUSED
cProc   TsGetLastUpdateDosh, <FAR, PUBLIC>, <BX, CX, SI, DI>
parmW   <dosh>
cBegin  TsGetLastUpdateDosh
        mov     bx, dosh
        mov     ax, 5700h
        int     21h
        jnc     lblSuccess
        mov     ax, -1
        mov     dx, ax
        jmp     lblDone
lblSuccess:
        mov     ax, cx
lblDone:
cEnd    TsGetLastUpdateDosh
endif ;NOTUSED


ifdef NOTUSED
;-----------------------------------------------------------------------------
;
;   BOOL FCreateTempFileLsz(lszPath, lpdosh)
;       Creates a file in the path specified in lszPath.
;       If successful, it will return TRUE and a dos-handle in
;       *lpdosh.  If it failed, it will return FALSE and the
;       error code returned by DOS in *lpdosh. The name file name
;       is put in *lszPath.  lszPath must have at lease 13 bytes of
;       memory behind the end of the string.
;       This function require DOS 3.1 or higher.
;
;-----------------------------------------------------------------------------
; %%Function:FCreateTempFileLsz %%Owner:NOTUSED
cProc   FCreateTempFileLsz, <FAR, PUBLIC>, <BX, CX, DX, DS>
parmD   <lszPath>
parmD   <lpdosh>
cBegin  FCreateTempFileLsz
        lds     dx, lszPath
        xor     cx, cx                  ; Normal file
        mov     ax, 5a00h               ; ah = 5ah
        int     21h
        lds     bx, lpdosh              ; Carry flag is not affected.
        mov     [bx], ax                ; ditto
        jc      lblEr
        mov     ax, 0001h
        jmp     lblFCTEnd
lblEr:  xor     ax, ax
lblFCTEnd:
cEnd    FCreateTempFileLsz
endif



ifndef	PMWORD
ifdef DEBUG
cProc   BreakPoint, <FAR, PUBLIC>
cBegin  BreakPoint
        int     3
cEnd    BreakPoint
endif	; DEBUG
endif	; PMWORD

;-------------------------------------------------------------------------
;	PnWhoseFcGEFc(fc)					GregC
;-------------------------------------------------------------------------
; %%Function:PnWhoseFcGEFc %%Owner:peterj
cProc	PnWhoseFcGEFc,<PUBLIC,FAR,ATOMIC>,<>
	ParmD	fc
; /* P N  W H O S E  ... */
; native PN PnWhoseFcGEFc(fc)
; FC	  fc;
; {
cBegin
;	  return ((fc + (cbSector - 1)) >> shftSector);
	mov	ax,[OFF_fc]
	mov	dx,[SEG_fc]
	add	ax,cbSector - 1
	adc	dx,0

; ASSUME shftSector == 9
        errnz   <shftSector - 9>
        shr     dx,1
        rcr     ax,1
	mov	al,ah
	mov	ah,dl
; }
cEnd


ifndef PMWORD
;
;  Returns TRUE if file is on a network
;
; %%Function:FFileRemote %%Owner:peterj
cProc	FFileRemote,<PUBLIC,FAR>
	ParmW	pFile		; pointer to file to check
cBegin
	mov	bx,pFile
	mov	bl,[bx]
	or	bl,20h
	sub	bl,'a'-1
	mov	ax,4409H
	int	21H
	mov	ax,0
	jc	gdax
	test	dh,10H
	jz	gdax
	inc	ax
gdax:
cEnd

else	; PMWORD
;
;  Returns TRUE if osfn is on a network
;
cProc	FOsfnRemote,<PUBLIC,FAR>
	ParmW	osfn
	LocalW	fsType
	LocalW	usDeviceAttr
cBegin
	lea	ax, fsType
	farPtr	lpfsType, ds, ax
	lea	bx, usDeviceAttr
	farPtr	lpusDeviceAttr, ds, bx
	cCall	DosQHandType, <osfn, lpfsType, lpusDeviceAttr>
	or	ax, ax
	mov	ax, 0	; assume error 
	jnz	FRRet
	test	fsType, 8000h
	jz	FRRet	; jump if not net drive 
	inc	ax	; net drive 
FRRet:	
cEnd
endif	; PMWORD

ifndef PMWORD
ifndef FAST

; Dummies used for the toolbox (to avoid mass compiles)

PUBLIC ToolboxDummy1
ToolboxDummy1 LABEL FAR

PUBLIC ToolboxDummy2
ToolboxDummy2 LABEL FAR

PUBLIC ToolboxDummy3
ToolboxDummy3 LABEL FAR

PUBLIC ToolboxDummy4
ToolboxDummy4 LABEL FAR

PUBLIC ToolboxDummy5
ToolboxDummy5 LABEL FAR

PUBLIC ToolboxDummy6
ToolboxDummy6 LABEL FAR

PUBLIC ToolboxDummy7
ToolboxDummy7 LABEL FAR

PUBLIC ToolboxDummy8
ToolboxDummy8 LABEL FAR

cProc ToolboxDummyEngine,<PUBLIC,FAR>
cBegin
    int 3
cEnd

endif ;FAST

endif ; PMWORD
sEnd        file2

            END
