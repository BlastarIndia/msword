;
; struct BMC definition
;
caBmc		    equ [00000h]
hbmBmc		    equ [word ptr 0000Ah]
flmBmc		    equ [word ptr 0000Ch]
lcbBmc		    equ [dword ptr 0000Eh]
cbBmcMin		equ 00012h

;
; struct BMI definition
;
hbmBmi		    equ [word ptr 00000h]
dxpBmi		    equ [word ptr 00002h]
dypBmi		    equ [word ptr 00004h]
fGrayBmi	    equ [byte ptr 00006h]
maskFGrayBmi	    equ 001h
fDiscardableBmi     equ [byte ptr 00006h]
maskFDiscardableBmi equ 002h
grpfSpareBmi	    equ [byte ptr 00006h]
maskgrpfSpareBmi    equ 0FCh
idcbMaxBmi	    equ [byte ptr 00007h]
dxpEachBmi	    equ [word ptr 00008h]

;
; struct BMIB definition
;
errnz	<cchSearchMax - 257>
rgwSearchBmib		equ  [word ptr 00000h]
rgwOppCaseBmib		equ  [word ptr 00202h]
cwSearchBmib		equ  [word ptr 00404h]
fNotPlainBmib		equ  [word ptr 00406h]
mpchdcpBmib		equ  [byte ptr 00408h]
fChTableInPatternBmib	equ  [word ptr 00508h]
cbBmibMin		equ  0050Ah

;
; struct BPTB definition
;
ibpMaxBptb          	equ  [word ptr 00000h]
ibpMacBptb          	equ  [word ptr 00002h]
iibpHashMaxBptb     	equ  [word ptr 00004h]
hprgibpHashBptb     	equ  [dword ptr 00006h]
OFF_hprgibpHashBptb	equ  [word ptr 00006h]
SB_hprgibpHashBptb	equ  [word ptr 00008h]
hpmpibpbpsBptb		equ  [dword ptr 0000Ah]
OFF_hpmpibpbpsBptb	equ  [word ptr 0000Ah]
SB_hpmpibpbpsBptb	equ  [word ptr 0000Ch]
hprgbpExtBptb		equ  [dword ptr 0000Eh]
OFF_hprgbpExtBptb	equ  [word ptr 0000Eh]
SB_hprgbpExtBptb	equ  [word ptr 00010h]
cbBpBptb		equ  [word ptr 00012h]
cbpChunkBptb		equ  [word ptr 00014h]
tsMruBpsBptb        	equ  [word ptr 00016h]
cqbpspnBptb		equ  [word ptr 00018h]
hpmpispntsBptb		equ  [dword ptr 0001Ah]
OFF_hpmpispntsBptb	equ  [word ptr 0001Ah]
SB_hpmpispntsBptb	equ  [word ptr 0001Ch]
cbBptbMin		equ  0001Eh

;
; struct BPS definition
;
pnBps           equ     [word ptr 00000h]
fDirtyBps       equ     [byte ptr 00002h]
fnBps           equ     [byte ptr 00003h]
tsBps           equ     [word ptr 00004h]
ibpHashNextBps  equ     [word ptr 00006h]
cbBpsMin        equ     00008h
cbBPS           equ     (00008h)

;
; struct BRC definition
;
brcBrc			equ	[word ptr 00000h]
brcBaseBrc		equ	[word ptr 00000h]
maskBrcBaseBrc		equ	001FFh
dxpLine2WidthBrc	equ	[word ptr 00000h]
maskDxpLine2WidthBrc	equ	00007h
dxpSpaceBetweenBrc	equ	[word ptr 00000h]
maskSpaceBetweenBrc	equ	00038h
dxpLine1WidthBrc	equ	[word ptr 00000h]
maskDxpLine1WidthBrc	equ	001C0h
dxpSpaceBrc		equ	[byte ptr 00001h]
maskDxpSpaceBrc 	equ	03Eh
fShadowBrc		equ	[byte ptr 00001h]
maskFShadowBrc		equ	040h
cbBrcMin		equ	00002h

;
; struct CA definition
;
cpFirstCa       equ     [dword ptr 00000h]
LO_cpFirstCa    equ     [word ptr 00000h]
HI_cpFirstCa    equ     [word ptr 00002h]
cpLimCa         equ     [dword ptr 00004h]
LO_cpLimCa      equ     [word ptr 00004h]
HI_cpLimCa      equ     [word ptr 00006h]
docCa           equ     [word ptr 00008h]
cbCaMin         equ     0000Ah
cwCA            equ     (0000Ah/00002h)

;
; struct CHP definition
;    WARNING: the CHP is a field of the CHR and FKPD, change them too!
fBoldChp	    equ     [byte ptr 00000h]
maskFBoldChp	    equ     001h
fItalicChp	    equ     [byte ptr 00000h]
maskFItalicChp	    equ     002h
fStrikeChp	    equ     [byte ptr 00000h]
maskFStrikeChp	    equ     004h
fOutlineChp	    equ     [byte ptr 00000h]
maskFOutlineChp     equ     008h
fFldVanishChp	    equ     [byte ptr 00000h]
maskFFldVanishChp   equ     010h
fSmallCapsChp	    equ     [byte ptr 00000h]
maskFSmallCapsChp   equ     020h
fCapsChp	    equ     [byte ptr 00000h]
maskFCapsChp	    equ     040h
fVanishChp	    equ     [byte ptr 00000h]
maskFVanishChp	    equ     080h
fRMarkChp	    equ     [byte ptr 00001h]
maskFRMarkChp	    equ     001h
fSpecChp	    equ     [byte ptr 00001h]
maskFSpecChp	    equ     002h
fsIcoChp	    equ     [byte ptr 00001h]
maskFsIcoChp	    equ     004h
fsFtcChp	    equ     [byte ptr 00001h]
maskFsFtcChp	    equ     008h
fsHpsChp	    equ     [byte ptr 00001h]
maskFsHpsChp	    equ     010h
fsKulChp	    equ     [byte ptr 00001h]
maskFsKulChp	    equ     020h
fsPosChp	    equ     [byte ptr 00001h]
maskFsPosChp	    equ     040h
fsSpaceChp	    equ     [byte ptr 00001h]
maskFsSpaceChp	    equ     080h
ftcChp		    equ     [word ptr 00002h]
hpsChp		    equ     [byte ptr 00004h]
hpsPosChp	    equ     [byte ptr 00005h]
qpsSpaceChp	    equ     [byte ptr 00006h]
maskQpsSpaceChp     equ     03fh
icoChp		    equ     [byte ptr 00007h]
maskIcoChp	    equ     00Fh
kulChp		    equ     [byte ptr 00007h]
maskKulChp	    equ     070h
ibitKulChp	    equ     4
fSysVanishChp	    equ     [byte ptr 00007h]
maskFSysVanishChp   equ     080h

fcPicChp	    equ     [dword ptr 00008h]
LO_fcPicChp	    equ     [word ptr 00008h]
HI_fcPicChp	    equ     [word ptr 0000Ah]
fnPicChp	    equ     [byte ptr 0000Bh]
docPicChp	    equ     [word ptr 00008h]
fDirtyChp	    equ     [byte ptr 0000Bh]
maskFDirtyChp	    equ     080h
hpsLargeChp	    equ     [word ptr 00008h]

cbChpMin	    equ     0000Ch
cbCHP		    equ     0000Ch
cwCHP		    equ     (0000Ch/00002h)
cwCHPBase	    equ     4
maskFsChp	    equ     0FC00h

;
; CHRM definitions
;
cbCHRE		equ	2

chrmChr         equ     [byte ptr 00000h]
ichChr          equ     [byte ptr 00001h]
chpChr          equ     [00002h]
fcidChr         equ     [dword ptr 0000Eh]
LO_fcidChr              equ     [word ptr 0000Eh]
HI_fcidChr              equ     [word ptr 00010h]
cbChrMin        equ     00012h
cbCHR           equ     00012h
cwCHR           equ     (00012h/00002h)

chrmChrt        equ     [byte ptr 00000h]
ichChrt         equ     [byte ptr 00001h]
chChrt          equ     [byte ptr 00002h]
tlcChrt         equ     [byte ptr 00003h]
cbChrtMin       equ     00004h
cbCHRT          equ     00004h

chrmChrv        equ     [byte ptr 00000h]
ichChrv         equ     [byte ptr 00001h]
dcpChrv         equ     [dword ptr 00002h]
LO_dcpChrv              equ     [word ptr 00002h]
HI_dcpChrv              equ     [word ptr 00004h]
cbChrvMin       equ     00006h
cbCHRV          equ     00006h

chrmChrf        equ     [byte ptr 00000h]
ichChrf         equ     [byte ptr 00001h]
dxpChrf         equ     [word ptr 00002h]
dypChrf         equ     [word ptr 00004h]
fLineChrf       equ     [word ptr 00006h]
cbChrfMin       equ     00008h
cbCHRF          equ     00008h

chrmChrdf	equ	[byte ptr 00000h]
ichChrdf	equ	[byte ptr 00001h]
fltChrdf	equ	[word ptr 00002h]
wChrdf		equ	[word ptr 00004h]
w2Chrdf	        equ	[word ptr 00006h]
lChrdf		equ	[dword ptr 00008h]
LO_lChrdf		equ	[word ptr 00008h]
HI_lChrdf		equ	[word ptr 0000Ah]
dxpChrdf	equ	[word ptr 0000Ch]
dypChrdf	equ	[word ptr 0000Eh]
dcpChrdf	equ	[dword ptr 00010h]
LO_dcpChrdf	equ	[word ptr 00010h]
HI_dcpChrdf	equ	[word ptr 00012h]
cbChrdfMin	equ	00014h
cbCHRDF		equ	00014h

chrmChrfg	equ	[byte ptr 00000h]
ichChrfg	equ	[byte ptr 00001h]
fltChrfg	equ	[word ptr 00002h]
dbchrChrfg	equ	[word ptr 00004h]
dxpChrfg	equ	[word ptr 00006h]
dcpChrfg	equ	[dword ptr 00008h]
LO_dcpChrfg	equ	[word ptr 00008h]
HI_dcpChrfg	equ	[word ptr 0000Ah]
cbChrfgMin	equ	0000Ch
cbCHRFG		equ	0000Ch

chrmChp         equ     cbCHR
chrmTab         equ     cbCHRT
chrmVanish      equ     cbCHRV
chrmFormula     equ     cbCHRF
chrmDisplayField        equ     cbCHRDF
chrmFormatGroup equ     cbCHRFG
chrmEnd         equ     cbCHRE

;
; struct CR definition
;
cpCr		equ	[dword ptr 00000h]
LO_cpCr		equ	[word ptr 00000h]
HI_cpCr		equ	[word ptr 00002h]
ccpCr		equ	[word ptr 00004h]
cbCrMin		equ	00006h

;
; struct DBS definition
;
nDbsuVerDbs		equ	[word ptr 00000h]
fCkDocDbs		equ	[byte ptr 00002h]
fCkFnDbs		equ	[byte ptr 00003h]
fCkWwDbs		equ	[byte ptr 00004h]
fCkBptbDbs		equ	[byte ptr 00005h]
fCkFkpDbs		equ	[byte ptr 00006h]
fCkPlcbteDbs		equ	[byte ptr 00007h]
fCkFliDbs		equ	[byte ptr 00008h]
fCkSccDbs		equ	[byte ptr 00009h]
fCkPrmDbs		equ	[byte ptr 0000Ah]
fCkDlgDbs		equ	[byte ptr 0000Bh]
fCkHeapDbs		equ	[byte ptr 0000Ch]
fShakeHeapDbs		equ	[byte ptr 0000Dh]
fCkStructDbs		equ	[byte ptr 0000Eh]
fCkFontDbs		equ	[byte ptr 0000Fh]
fCkMwdDbs		equ	[byte ptr 00010h]
fCk16Dbs		equ	[byte ptr 00011h]
fCkFldDelDbs		equ	[byte ptr 00012h]
fCkFldIdleDbs		equ	[byte ptr 00013h]
fCkLHRUpdDbs		equ	[byte ptr 00014h]
fCkScrollDbs		equ	[byte ptr 00015h]
fCkPaphDbs		equ	[byte ptr 00016h]
fCkTextDbs		equ	[byte ptr 00017h]
fCkTlbxDbs		equ	[byte ptr 00018h]
fCk24Dbs		equ	[byte ptr 00019h]
fCk25Dbs		equ	[byte ptr 0001Ah]
fCk26Dbs		equ	[byte ptr 0001Bh]
fCk27Dbs		equ	[byte ptr 0001Ch]
fCk28Dbs		equ	[byte ptr 0001Dh]
fCk29Dbs		equ	[byte ptr 0001Eh]
fCk30Dbs		equ	[byte ptr 0001Fh]
fCk31Dbs		equ	[byte ptr 00020h]
fCk32Dbs		equ	[byte ptr 00021h]
fCk33Dbs		equ	[byte ptr 00022h]
fCk34Dbs		equ	[byte ptr 00023h]
fCk35Dbs		equ	[byte ptr 00024h]
fCk36Dbs		equ	[byte ptr 00025h]
fCommSzDbs		equ	[byte ptr 00026h]
fMoreCommDbs		equ	[byte ptr 00027h]
fRipToDbgScreenDbs	equ	[byte ptr 00028h]
fRetryDefDbs		equ	[byte ptr 00029h]
fReportsDbs		equ	[byte ptr 0002Ah]
fRipOnErrorDbs		equ	[byte ptr 0002Bh]
fFailInMcrDbs		equ	[byte ptr 0002Ch]
fReportHeapDbs		equ	[byte ptr 0002Dh]
fFixedMemDbs		equ	[byte ptr 0002Eh]
fOutsideCmdDbs		equ	[byte ptr 0002Fh]
fHeapActDbs		equ	[byte ptr 00030h]
fWinActDbs		equ	[byte ptr 00031h]
fTraceGDIDbs		equ	[byte ptr 00032h]
fNoCoreLoadDbs		equ	[byte ptr 00033h]
fNoInfoRTFDbs		equ	[byte ptr 00034h]
fFail8087Dbs		equ	[byte ptr 00035h]
fCommDde1Dbs		equ	[byte ptr 00036h]
fCommDde2Dbs		equ	[byte ptr 00037h]
fNoEmmDbs		equ	[byte ptr 00038h]
fBreakOnFailDbs 	equ	[byte ptr 00039h]
fNoFillBlockDbs 	equ	[byte ptr 0003Ah]
fCallSetFlmDbs		equ	[byte ptr 0003Bh]
fCommPrintDbs		equ	[byte ptr 0003Ch]
fCoverActiveDbs		equ	[byte ptr 0003Dh]
fShowDirtyDbs		equ	[byte ptr 0003Eh]
fCompressOftDbs 	equ	[byte ptr 0003Fh]
fTrackGDIHandlesDbs	equ	[byte ptr 00040h]
fUseTextOutDbs		equ	[byte ptr 00041h]
fShowLbsDbs		equ	[byte ptr 00042h]
fSmallFileCacheDbs	equ	[byte ptr 00043h]
fNoRtfConvDbs		equ	[byte ptr 00044h]
fAllGraphicsDbs 	equ	[byte ptr 00045h]
fExtTextOutNullDbs	equ	[byte ptr 00046h]
fDumpMessagesDbs	equ	[byte ptr 00047h]
fPref35Dbs		equ	[byte ptr 00048h]
fPref36Dbs		equ	[byte ptr 00049h]
fUseC01Dbs		equ	[byte ptr 0004Ah]
fUseC02Dbs		equ	[byte ptr 0004Bh]
fUseC03Dbs		equ	[byte ptr 0004Ch]
fUseC04Dbs		equ	[byte ptr 0004Dh]
fUseC05Dbs		equ	[byte ptr 0004Eh]
fUseC06Dbs		equ	[byte ptr 0004Fh]
fUseC07Dbs		equ	[byte ptr 00050h]
fUseC08Dbs		equ	[byte ptr 00051h]
fUseC09Dbs		equ	[byte ptr 00052h]
fUseC10Dbs		equ	[byte ptr 00053h]
fUseC11Dbs		equ	[byte ptr 00054h]
fUseC12Dbs		equ	[byte ptr 00055h]
fUseC13Dbs		equ	[byte ptr 00056h]
fUseC14Dbs		equ	[byte ptr 00057h]
fUseC15Dbs		equ	[byte ptr 00058h]
fUseC16Dbs		equ	[byte ptr 00059h]
fUseC17Dbs		equ	[byte ptr 0005Ah]
fUseC18Dbs		equ	[byte ptr 0005Bh]
fUseC19Dbs		equ	[byte ptr 0005Ch]
fUseC20Dbs		equ	[byte ptr 0005Dh]
fUseC21Dbs		equ	[byte ptr 0005Eh]
fUseC22Dbs		equ	[byte ptr 0005Fh]
fUseC23Dbs		equ	[byte ptr 00060h]
fUseC24Dbs		equ	[byte ptr 00061h]
fUseC25Dbs		equ	[byte ptr 00062h]
fUseC26Dbs		equ	[byte ptr 00063h]
fUseC27Dbs		equ	[byte ptr 00064h]
fUseC28Dbs		equ	[byte ptr 00065h]
fUseC29Dbs		equ	[byte ptr 00066h]
fUseC30Dbs		equ	[byte ptr 00067h]
fUseC31Dbs		equ	[byte ptr 00068h]
fUseC32Dbs		equ	[byte ptr 00069h]
fUseC33Dbs		equ	[byte ptr 0006Ah]
fUseC34Dbs		equ	[byte ptr 0006Bh]
fUseC35Dbs		equ	[byte ptr 0006Ch]
fUseC36Dbs		equ	[byte ptr 0006Dh]
fDisableAsmDbs		equ	[byte ptr 0006Dh]   ;note same byte as above
grpfScribbleDbs equ	[word ptr 0006Eh]
fSbDdsOnly	equ	[word ptr 00070h]
fCmdOnly	equ	[word ptr 00072h]
rgREVIEWDbs	equ	[00074h]
cLmemSucceedDbs equ	[word ptr 00078h]
cLmemFailDbs	equ	[word ptr 0007Ah]
cWinSucceedDbs	equ	[word ptr 0007Ch]
cWinFailDbs	equ	[word ptr 0007Eh]
wREVIEW2Dbs	equ	[word ptr 00080h]
fQuicksaveDbs	equ	[word ptr 00082h]
fInfoAssertDbs	equ	[word ptr 00084h]
fSprmTablesCheckedDbs	equ	[word ptr 00086h]
StatLineDbs	equ	[word ptr 00088h]
dbcsFliDbs	equ	[0008Ah]
dbcsDisplayDbs	equ	[000ACh]
dbcsLrDbs	equ	[000CEh]
cdlInvalDbs	equ	[word ptr 000F0h]
fScanDbs	equ	[word ptr 000F2h]
jc1Dbs		equ	[word ptr 000F4h]
jnSccDbs	equ	[word ptr 000F6h]
fJoinBelowDbs	equ	[word ptr 000F8h]
fJoinAboveDbs	equ	[word ptr 000FAh]
fcCurDbs	equ	[dword ptr 000FCh]
LO_fcCurDbs	equ	[word ptr 000FCh]
HI_fcCurDbs	equ	[word ptr 000FEh]
osfnDbs 	equ	[word ptr 00100h]
stRoutineNameDbs	equ	[00102h]
stFileDbs	equ	[00120h]
fFlushFailedDbs equ	[word ptr 0013Eh]
cbHeapWantedCmdDbs	equ	[word ptr 00140h]
hDebugDbs	equ	[word ptr 00142h]
cbDbsMin	equ	00144h

;
; struct DCIB definition
;
ropBitBltDcib	    equ     [dword ptr 00000h]
LO_ropBitBltDcib    equ     [word ptr 00000h]
HI_ropBitBltDcib    equ     [word ptr 00002h]
ropEraseDcib	    equ     [dword ptr 00004h]
LO_ropEraseDcib     equ     [word ptr 00004h]
HI_ropEraseDcib     equ     [word ptr 00006h]
cbDcibMin	    equ     00008h

;
; struct DOD definition
;
fDocDod 		equ	[byte ptr 00000h]
maskFDocDod		equ	01h
fDotDod 		equ	[byte ptr 00000h]
maskFDotDod		equ	02h
fGlsyDod		equ	[byte ptr 00000h]
maskFGlsyDod		equ	04h
fAtnDod 		equ	[byte ptr 00001h]
maskFAtnDod		equ	01h
fMcrDod 		equ	[byte ptr 00001h]
maskFMcrDod		equ	02h
fHdrDod 		equ	[byte ptr 00001h]
maskFHdrDod		equ	04h
fFtnDod 		equ	[byte ptr 00001h]
maskFFtnDod		equ	08h
fSDocDod		equ	[byte ptr 00001h]
maskFSDocDod		equ	10h
fDispHdrDod		equ	[byte ptr 00001h]
maskFDispHdrDod 	equ	20h
fMotherDod		equ	[byte ptr 00000h]
fShortDod		equ	[byte ptr 00001h]
dkDod			equ	[word ptr 00000h]
fDirtyDod		equ	[byte ptr 00002h]
maskFDirtyDod		equ	01h
fStshDirtyDod		equ	[byte ptr 00002h]
maskFStshDirtyDod       equ     02h
fLRDirtyDod		equ	[byte ptr 00002h]
maskFLRDirtyDod 	equ	04h
fOutlineDirtyDod	equ	[byte ptr 00002h]
maskFOutlineDirtyDod    equ     08h
fMotherStshDod		equ	[byte ptr 00002h]
maskFMotherStshDod      equ     10h
fFormattedDod		equ	[byte ptr 00002h]
maskFFormattedDod	equ	20h
fSeaDod 		equ	[byte ptr 00002h]
maskFSeaDod		equ	40h
fRepagDod		equ	[byte ptr 00002h]
maskFRepagDod		equ	80h
fLockForEditDod 	equ	[byte ptr 00003h]
maskFLockForEditDod	equ	01h
fPromptSI		equ	[byte ptr 00003h]
maskFPromptSIDod	equ	02h
fGuardedDod		equ	[byte ptr 00003h]
maskFGuardedDod 	equ	04h
fSedMacEopCopiedDod	equ	[byte ptr 00003h]
maskFSedMacEopCopiedDod equ	08h
fEnvDirtyDod		equ	[byte ptr 00003h]
maskFEnvDirtyDod	equ	10h
fDocClosedDod		equ	[byte ptr 00003h]
maskFDocClosedDod	equ	20h
fFldNestedValidDod	equ	[byte ptr 00003h]
maskFFldNestedValidDod	equ	40h
fDefaultFormatDod	equ	[byte ptr 00003h]
maskFDefaultFormatDod	equ	80h
fHdrSameDod		equ	[byte ptr 00004h]
maskFHdrSameDod 	equ	01h
fMayHavePicDod		equ	[byte ptr 00004h]
maskFMayHavePicDod	equ	02h
wwDispDod		equ	[byte ptr 00006h]
fnDod			equ	[byte ptr 00007h]
cpMacDod		equ	[dword ptr 00008h]
LO_cpMacDod		equ	[word ptr 00008h]
HI_cpMacDod		equ	[word ptr 0000Ah]
docDod			equ	[word ptr 0000Ch]
hplcpcdDod		equ	[word ptr 0000Eh]
hplcpheDod		equ	[word ptr 00010h]
hplcpgdDod	        equ	[word ptr 00012h]
hplcfldDod		equ	[word ptr 00014h]
hplcfndDod		equ	[word ptr 00016h]
hplcfndDod		equ	[word ptr 00016h]
hplchddDod		equ	[word ptr 00016h]
hplcglsyDod		equ	[word ptr 00016h]
hplcmcrDod		equ	[word ptr 00016h]
hplcdliDod		equ	[word ptr 00016h]
hplcandDod		equ	[word ptr 00016h]
crefLockDod 		equ	[word ptr 00018h]
docHdrDod	        equ	[word ptr 0001Ah]
drpFtnDod	        equ	[dword ptr 0001Ch]
LO_drpFtnDod	        equ	[word ptr 0001Ch]
HI_drpFtnDod	        equ	[word ptr 0001Eh]
docFtnDod	        equ	[word ptr 0001Ch]
hplcfrdDod	        equ	[word ptr 0001Eh]
;end of short dod

drpAtnDod	        equ	[dword ptr 00020h]
LO_drpAtnDod	        equ	[word ptr 00020h]
HI_drpAtnDod	        equ	[word ptr 00022h]
docAtnDod	        equ	[word ptr 00020h]
hplcatrdDod	        equ	[word ptr 00022h]
udtDod			equ	[byte ptr 00024h]
maskUdtDod		equ	007h
iInstanceDod		equ	[byte ptr 00024h]
maskIInstanceDod	equ	0F8h
hplcsedDod	        equ	[word ptr 00026h]
hplcpadDod	        equ	[word ptr 00028h]
stshDod		        equ	[0002Ah]
hsttbChpeDod	        equ	[word ptr 00034h]
hsttbPapeDod	        equ	[word ptr 00036h]
dttmOpenedDod	        equ	[dword ptr 00038h]
LO_dttmOpenedDod	equ	[word ptr 00038h]
HI_dttmOpenedDod	equ	[word ptr 0003Ah]
hsttbAssocDod	        equ	[word ptr 0003Ch]
hplcseaDod	        equ	[word ptr 0003Eh]
hplcbkfDod	        equ	[word ptr 00040h]
hplcbklDod	        equ	[word ptr 00042h]
hsttbBkmkDod	        equ	[word ptr 00044h]
hmpftcibstFontDod	equ	[word ptr 00046h]
ftcMacDod	        equ	[word ptr 00048h]
ftcMaxDod	        equ	[word ptr 0004Ah]
docDotDod	        equ	[word ptr 0004Ch]
hsttbGlsyDod	        equ	[word ptr 0004Ch]
docGlsyDod	        equ	[word ptr 0004Ch]
hkmpUserDod	        equ	[word ptr 0004Eh]
hmudUserDod	        equ	[word ptr 00050h]
docMcrDod	        equ	[word ptr 00052h]
lvlDod			equ	[byte ptr 00054h]
maskLvlDod		equ	00Fh
wREVIEW2Dod	        equ	[word ptr 00054h]
wREVIEW1Dod	        equ	[word ptr 00056h]
dopDod		        equ	[00058h]
cbDodMin		equ	0009Ah
cbDOD			equ	(0009Ah)
cwDOD		        equ	(cbDOD/00002h)
cbDODShort		equ	(00020h)
cwDODShort		equ	(cbDODShort/00002h)

;
; struct DR definition
;
drclDr			equ	[0000h]
xlDr			equ	[word ptr 00000h]
ylDr			equ	[word ptr 00002h]
dxlDr			equ	[word ptr 00004h]
dylDr			equ	[word ptr 00006h]
caDr			equ	[0008h]
cpFirstDr		equ	[dword ptr 00008h]
LO_cpFirstDr		equ	[word ptr 00008h]
HI_cpFirstDr		equ	[word ptr 0000Ah]
cpLimDr 		equ	[dword ptr 0000Ch]
LO_cpLimDr		equ	[word ptr 0000Ch]
HI_cpLimDr		equ	[word ptr 0000Eh]
docDr			equ	[word ptr 00010h]
dcpDependDr		equ	[byte ptr 00012h]
fDirtyDr		equ	[byte ptr 00013h]
maskfDirtyDr		equ	001h
fCpBadDr		equ	[byte ptr 00013h]
maskfCpBadDr		equ	002h
fLimSuspectDr		equ	[byte ptr 00013h]
maskfLimSuspectDr	equ	004h
fNoParaStartDr		equ	[byte ptr 00013h]
maskfNoParaStartDr	equ	008h
fIncompleteDr		equ	[byte ptr 00013h]
maskfIncompleteDr	equ	010h
fInTableDr		equ	[byte ptr 00013h]
maskfInTableDr		equ	020h
fBottomTableFrameDr	equ	[byte ptr 00013h]
maskfBottomTableFrameDr equ	040h
fForceWidthDr		equ	[byte ptr 00013h]
maskfForceWidthDr	equ	080h
hplcedlDr		equ	[word ptr 00014h]
dypAboveDr		equ	[word ptr 00016h]
dxpOutLeftDr		equ	[word ptr 00018h]
dxpOutRightDr		equ	[word ptr 0001Ah]
idrFlowDr		equ	[word ptr 0001Ch]
ccolM1Dr		equ	[byte ptr 0001Eh]
lrkDr			equ	[byte ptr 0001Fh]
fNoPrevDr		equ	[byte ptr 00020h]
maskfNoPrevDr		equ	001h
fConstrainLeftDr	equ	[byte ptr 00020h]
maskfConstrainLeftDr	equ	002h
fConstrainRightDr	equ	[byte ptr 00020h]
maskfConstrainRightDr	equ	004h
fNewColOnlyDr		equ	[byte ptr 00020h]
maskfNewColOnlyDr	equ	008h
fFatLineDr		equ	[byte ptr 00020h]
maskfFatLineDr		equ	010h
fCantGrowDr		equ	[byte ptr 00020h]
maskfCantGrowDr 	equ	020h
fHdrFtrDr		equ	[byte ptr 00020h]
maskfHdrFtrDr		equ	040h
fForceFirstRowDr	equ	[byte ptr 00020h]
maskfForceFirstRowDr	equ	080h
fRMarkDr	        equ	[byte ptr 00021h]
maskfRMarkDr	        equ	001h
fSpaceBeforeDr		equ	[byte ptr 00021h]
maskfSpaceBeforeDr	equ	002h
xwLimScrollDr		equ     [word ptr 00022h]
dxaBetweenDr            equ     [word ptr 00024h]
plcedlDr		equ	00026h
dxaDr			equ	[word ptr 0003Ah]
ihdtDr			equ	[word ptr 0003Ch]
cbDrMin 		equ	0003Eh

;
; struct DRF definition
;
hpldrDrf		equ	[word ptr 00000h]
idrDrf			equ	[word ptr 00002h]
drDrf			equ	00004h
pplcedlDrf		equ	[word ptr 00042h]
pdrfNextDrf		equ	[word ptr 00044h]
pdrfUsedDrf		equ	[word ptr 00046h]
wLastDrf		equ	[word ptr 00048h]
cbDrfMin		equ	0004Ah

;
; struct DOP definition
;    WARNING: the DOP is a field of the DOD, change it too!
fFacingPagesDop 	equ	[byte ptr 00000h]
maskFFacingPagesDop     equ     01h
fWidowControlDop	equ	[byte ptr 00000h]
maskFWidowControlDop    equ     02h
fpcDop			equ	[byte ptr 00000h]
maskFpcDop		equ	060h
ibitFpcDop		equ	5
fWideDop		equ	[byte ptr 00000h]
maskFWideDop		equ	080h
grpfIhdtDop		equ	[byte ptr 00001h]
fFtnRestartDop		equ	[byte ptr 00002h]
maskfFtnRestartDop	equ	001h
nFtnDop 		equ	[word ptr 00002h]
maskNFtnDop		equ	0FFFEh
ibitNFtnDop		equ	1
irmBarDop		equ	[byte ptr 00004h]
irmPropsDop		equ	[byte ptr 00005h]
maskIrmPropsDop 	equ	07Fh
fRevMarkingDop		equ	[byte ptr 00005h]
maskfRevMarkingDop	equ	080h
fBackupDop		equ	[byte ptr 00006h]
maskFBackupDop		equ	001h
fExactCWordsDop 	equ	[byte ptr 00006h]
maskFExactCWordsDop	equ	002h
fPagHiddenDop		equ	[byte ptr 00006h]
maskFPagHiddenDop	equ	004h
fPagResultsDop		equ	[byte ptr 00006h]
maskFPagResultsDop	equ	008h
fLockAtnDop		equ	[byte ptr 00006h]
maskFLockAtnDop 	equ	010h
fMirrorMarginsDop	equ	[byte ptr 00006h]
maskFMirrorMarginsDop	equ	020h
fSparesDop		equ	[word ptr 00008h]
yaPageDop		equ	[word ptr 0000Ah]
xaPageDop		equ	[word ptr 0000Ch]
dyaTopDop		equ	[word ptr 0000Eh]
dxaLeftDop		equ	[word ptr 00010h]
dyaBottomDop		equ	[word ptr 00012h]
dxaRightDop		equ	[word ptr 00014h]
dxaGutterDop		equ	[word ptr 00016h]
dxaTabDop		equ	[word ptr 00018h]
; 1 spare word
dxaHotZDop		equ	[word ptr 0001Ch]
rgwSpareDop		equ	[word ptr 0001Eh]
; 1 spare word
dttmCreatedDop		equ	[dword ptr 00022h]
LO_dttmCreatedDop	equ	[word ptr 00022h]
HI_dttmCreatedDop	equ	[word ptr 00024h]
dttmRevisedDop		equ	[dword ptr 00026h]
LO_dttmRevisedDop	equ	[word ptr 00026h]
HI_dttmRevisedDop	equ	[word ptr 00028h]
dttmLastPrintDop	equ	[dword ptr 002Ah]
LO_dttmLastPrintDop	equ	[word ptr 0002Ah]
HI_dttmLastPrintDop	equ	[word ptr 0002Ch]
nRevisionDop		equ	[word ptr 0002Eh]
tmEditedDop		equ	[dword ptr 00030h]
LO_tmEditedDop		equ	[word ptr 00030h]
HI_tmEditedDop		equ	[word ptr 00032h]
cWordsDop		equ	[dword ptr 00034h]
LO_cWordsDop		equ	[word ptr 00034h]
HI_cWordsDop		equ	[word ptr 00036h]
cChDop			equ	[dword ptr 00038h]
LO_cChDop		equ	[word ptr 00038h]
HI_cChDop		equ	[word ptr 0003Ah]
cPgDop			equ	[word ptr 0003Ch]
rgwSpareDocSumDop	equ	[word ptr 0003Eh]
cbDopMin		equ	00042h
cwDOP			equ	(00042h/00002h)
cbDOP			equ	(00042h)
;    WARNING: the DOP is a field of the DOD, change it too!


;
; struct DRC definition
;
xpDrc			equ	[word ptr 00000h]
ypDrc			equ	[word ptr 00002h]
dxpDrc			equ	[word ptr 00004h]
dypDrc			equ	[word ptr 00006h]
ptTopLeftDrc		equ	00000h
ptDxDyDrc		equ	00004h
xwDrc			equ	[word ptr 00000h]
ywDrc			equ	[word ptr 00002h]
dxwDrc			equ	[word ptr 00004h]
dywDrc			equ	[word ptr 00006h]
xlDrc			equ	[word ptr 00000h]
ylDrc			equ	[word ptr 00002h]
dxlDrc			equ	[word ptr 00004h]
dylDrc			equ	[word ptr 00006h]
xeDrc			equ	[word ptr 00000h]
yeDrc			equ	[word ptr 00002h]
dxeDrc			equ	[word ptr 00004h]
dyeDrc			equ	[word ptr 00006h]
cbDrcMin		equ	00008h

;
; struct DRDL definition
;
idrDrdl 		equ	[word ptr 00000h]
hpldrDrdl		equ	[word ptr 00002h]
dlDrdl			equ	[word ptr 00004h]
cbDrdlMin		equ	00006h

;
; struct EDL definition
;
grpfEdl 	    equ     [word ptr 00000h]
dcpDependEdl	    equ     [byte ptr 00000h]
dlkEdl		    equ     [byte ptr 00001h]
maskDlkEdl	    equ     007h
ibitDlkEdl	    equ     0
fEndEdl 	    equ     [byte ptr 00001h]
maskFEndEdl	    equ     001h
ibitFEndEdl	    equ     0
fEndPageEdl	    equ     [byte ptr 00001h]
maskFEndPageEdl     equ     002h
ibitFEndPageEdl     equ     1
fEndDrEdl	    equ     [byte ptr 00001h]
maskFEndDrEdl	    equ     004h
ibitFEndDrEdl	    equ     2
fDirtyEdl	    equ     [byte ptr 00001h]
maskFDirtyEdl	    equ     008h
ibitFDirtyEdl	    equ     3
fHasBitmapEdl	    equ     [byte ptr 00001h]
maskFHasBitmapEdl   equ     010h
ibitFHasBitmapEdl   equ     4
fRMarkEdl           equ     [byte ptr 00001h]
maskFRMarkEdl       equ     010h
ibitFRMarkEdl       equ     4
fTableDirtyEdl      equ     [byte ptr 00001h]
maskFTableDirtyEdl  equ     020h
ibitFTableDirtyEdl  equ     5
fColorMFPEdl	    equ     [byte ptr 00001h]
maskFColorMFPEdl    equ     040h
ibitFColorMFPEdl    equ     6
fNeedEnhanceEdl	    equ     [byte ptr 00001h]
maskFNeedEnhanceEdl equ     080h
ibitFNeedEnhanceEdl equ     7
hpldrEdl	    equ     [word ptr 00002h]
drcpEdl 	    equ     [00004h]
xpLeftEdl	    equ     [word ptr 00004h]
ypTopEdl	    equ     [word ptr 00006h]
dxpEdl		    equ     [word ptr 00008h]
dypEdl		    equ     [word ptr 0000Ah]
dcpEdl		    equ     [dword ptr 0000Ch]
LO_dcpEdl	    equ     [word ptr 0000Ch]
HI_dcpEdl	    equ     [word ptr 0000Eh]
cbEdlMin	    equ     00010h
cwEDL		    equ     (00010h/00002h)

;
; struct EFLT definition
;
fDeadEflt		equ	[byte ptr 00000h]
maskfDeadEflt		equ	001h
ibitfDeadEflt		equ	0
fLiveEflt		equ	[byte ptr 00000h]
maskfLiveEflt		equ	002h
ibitfLiveEflt		equ	1
fResultEflt		equ	[byte ptr 00000h]
maskfResultEflt		equ	004h
ibitfResultEflt		equ	2
fDisplayEflt		equ	[byte ptr 00000h]
maskfDisplayEflt	equ	008h
ibitfDisplayEflt	equ	3
fltgEflt		equ	[byte ptr 00000h]
maskfltgEflt		equ	060h
ibitfltgEflt		equ	5
frmEflt			equ	[byte ptr 00001h]
pfnCalcFuncEflt		equ	[word ptr 00002h]
cbEfltMin		equ	00004h


;
; struct ESPRM definition
;
fClobberEsprm           equ     [byte ptr 00000h]
maskfClobberEsprm       equ     001h
ibitfClobberEsprm       equ     0
bEsprm                  equ     [byte ptr 00000h]
maskbEsprm		equ	0FEh
ibitbEsprm              equ     1
spraEsprm               equ     [byte ptr 00001h]
maskspraEsprm		equ	01Fh
ibitspraEsprm           equ     0
sgcEsprm                equ     [byte ptr 00001h]
masksgcEsprm		equ	0E0h
ibitsgcEsprm            equ     5
cchEsprm                equ     [byte ptr 00002h]
maskCchEsprm		equ	00Fh
ibitCchEsprm		equ	0
dsprmEsprm              equ     [byte ptr 00002h]
maskdsprmEsprm		equ	0F0h
ibitdsprmEsprm          equ     4
fClobberExemptEsprm     equ     [byte ptr 00003h]
maskfClobberExemptEsprm equ     001h
ibitfClobberExemptEsprm equ     0
cbEsprmMin              equ     00004h

;
; struct ESTCP definition
;
stcNextEstcp		equ	[byte ptr 00000h]
stcBaseEstcp		equ	[byte ptr 00001h]
cbEstcpMin		equ	00002h

;
; struct FB definition
;
fInfoFb 		equ	[byte ptr 00000h]
fChpFb			equ	[byte ptr 00000h]
maskfChpFb		equ	001h
ibitfChpFb		equ	0
fPapFb			equ	[byte ptr 00000h]
maskfPapFb		equ	002h
ibitfPapFb		equ	1
fTextFb 		equ	[byte ptr 00000h]
maskfTextFb		equ	004h
ibitfTextFb		equ	2
chpFb			equ	[word ptr 00002h]
papFb			equ	[word ptr 0000Eh]
chpGrayFb		equ	[word ptr 0002Ch]
papGrayFb		equ	[word ptr 00038h]
cbFbMin 		equ	00056h

;
; struct FBB definition
;
cchMaxFbb		equ	[word ptr 00000h]
rgchFbb 		equ	[word ptr 00002h]
ccrMaxFbb		equ	[word ptr 00004h]
rgcrFbb 		equ	[word ptr 00006h]
cbFbbMin		equ	00008h

;
; struct FCB definition
;
fHasFibFcb              equ     [byte ptr 00000h]
maskFHasFibFcb          equ     001h
fRefDocScrapFcb         equ     [byte ptr 00000h]
maskFRefDocScrapFcb     equ     002h
fRefNonScrapFcb         equ     [byte ptr 00000h]
maskFRefNonScrapFcb     equ     004h
fDictFcb                equ     [byte ptr 00000h]
maskFDictFcb            equ     008h
fGlsyFcb                equ     [byte ptr 00000h]
maskFGlsyFcb            equ     010h
fDocFcb                 equ     [byte ptr 00000h]
maskFDocFcb             equ     020h
fTempFcb                equ     [byte ptr 00000h]
maskFTempFcb            equ     040h
fReadOnlyFcb            equ     [byte ptr 00000h]
maskFReadOnlyFcb        equ     080h
fCompoundFcb		equ	[byte ptr 00001h]
maskFCompoundFcb	equ	001h
fOpenedFcb              equ     [byte ptr 00001h]
maskFOpenedFcb          equ     002h
fDMEnumFcb              equ     [byte ptr 00001h]
maskFDMEnumFcb          equ     004h
fForeignFormatFcb	equ     [byte ptr 00001h]
maskFForeignFormatFcb	equ     008h
fDirtyFcb		equ	[byte ptr 00001h]
maskFDirtyFcb		equ	010h
cbMacFcb		equ	[dword ptr 00002h]
LO_cbMacFcb		equ	[word ptr 00002h]
HI_cbMacFcb		equ	[word ptr 00004h]
fcMacFileFcb		equ	[dword ptr 00006h]
LO_fcMacFileFcb		equ	[word ptr 00006h]
HI_fcMacFileFcb		equ	[word ptr 00008h]
fcPosFcb		equ	[dword ptr 0000Ah]
LO_fcPosFcb		equ	[word ptr 0000Ah]
HI_fcPosFcb		equ	[word ptr 0000Ch]
osfnFcb 		equ	[word ptr 0000Eh]
tsFcb			equ	[word ptr 00010h]
pnXLimitFcb		equ	[word ptr 00012h]
hplcbteChpFcb		equ	[word ptr 00014h]
hplcbtePapFcb		equ	[word ptr 00016h]
ofhFcb			equ	[00018h]
pnChpHintFcb		equ	[word ptr 0020h]
ifcChpHintFcb		equ	[word ptr 0022h]
pnPapHint1Fcb		equ	[word ptr 0024h]
ifcPapHint1Fcb		equ	[word ptr 0026h]
pnPapHint2Fcb		equ	[word ptr 0028h]
ifcPapHint2Fcb		equ	[word ptr 002Ah]
fcMaxTextFcb	equ	[dword ptr 0002Ch]
LO_fcMaxTextFcb	equ	[word ptr 0002Ch]
HI_fcMaxTextFcb	equ	[word ptr 0002Eh]
nFibFcb 	equ	[word ptr 00030h]
wREVIEW1Fcb	equ	[word ptr 00032h]
stFileFcb	equ	[word ptr 00034h]
cbFcbMin	equ	00036h
cwFCB		equ	(00036h/00002h)

;
; struct FCE definition
;
fcidActualFce           equ     [dword ptr 00000h]
LO_fcidActualFce        equ     [word ptr 00000h]
HI_fcidActualFce        equ     [word ptr 00002h]
dxpOverhangFce          equ     [word ptr 00004h]
dypAscentFce            equ     [word ptr 00006h]
dypDescentFce           equ     [word ptr 00008h]
dypXtraAscentFce       equ     [word ptr 0000Ah]
fPrinterFce             equ     [byte ptr 0000Ch]
maskfPrinterFce 	equ	001h
shiftfPrinterFce	equ	0
fVisiBadFce             equ     [byte ptr 0000Ch]
maskfVisiBadFce 	equ	002h
shiftfVisiBadFce	equ	1
fFixedPitchFce		equ     [byte ptr 0000Ch]
maskfFixedPitchFce	equ	004h
fPrvwFce		equ	[byte ptr 0000Ch]
maskfPrvwFce		equ	008h
shiftfPrvwFce		equ	3
fGraphicsFce		equ	[byte ptr 0000Ch]
maskfGraphicsFce 	equ	080h
shiftfGraphicsFce     	equ	7
hqrgdxpFce		equ	[dword ptr 0000Eh]
LO_hqrgdxpFce		equ	[word ptr 0000Eh]
HI_hqrgdxpFce		equ	[word ptr 00010h]
dxpWidthFce             equ     [word ptr 0000Eh]
fcidRequestFce          equ     [dword ptr 00012h]
LO_fcidRequestFce       equ     [word ptr 00012h]
HI_fcidRequestFce       equ     [word ptr 00014h]
pfceNextFce             equ     [word ptr 00016h]
pfcePrevFce             equ     [word ptr 00018h]
hfontFce                equ     [word ptr 0001Ah]
cbFceMin                equ     0001Ch

;
; struct FCID definition
;
lFcid			  equ	  [dword ptr 00000h]
LO_lFcid		  equ	  [word ptr 00000h]
HI_lFcid		  equ	  [word ptr 00002h]
wPropsFcid		  equ	  [word ptr 00000h]
wExtraFcid		  equ	  [word ptr 00002h]
fBoldFcid		  equ	  [byte ptr 0000h]
maskFBoldFcid		  equ	  01h
shiftFBoldFcid		  equ	  00h
fItalicFcid		  equ	  [byte ptr 0000h]
maskFItalicFcid 	  equ	  02h
shiftFItalicFcid	  equ	  01h
fStrikeFcid		  equ	  [byte ptr 0000h]
maskFStrikeFcid 	  equ	  04h
shiftFStrikeFcid	  equ	  02h
kulFcid 		  equ	  [byte ptr 0000h]
maskKulFcid		  equ	  038h
shiftKulFcid		  equ	  03h
ibitKulFcid		  equ	  3
prqFcid			  equ	  [byte ptr 0000h]
maskPrqFcid		  equ 	  0C0h
shiftPrqFcid		  equ     6
hpsFcid 		  equ	  [byte ptr 0001h]
ibstFontFcid		  equ	  [byte ptr 0002h]
cbFcid			  equ	  00004h

;
; struct FFN definition
;
cbFfnM1Ffn              equ     [byte ptr 00000h]
ffidFfn                 equ     [byte ptr 00001h]
prqFfn			equ	[byte ptr 00001h]
maskPrqFfn		equ	03h
fRasterFfn		equ	[byte ptr 00001h]
maskFRasterFfn		equ	08h
fGraphicsFfn		equ	[byte ptr 00001h]
maskfGraphicsFfn	equ	080h
szFfn                   equ     [00002h]
cbFfnMin                equ     00003h
cbFfnLast               equ     szFfn + LF_FACESIZE + 1	; +1 for chs at end 

;
;       struct FFS definition
;
fFormattingFfs          equ [word ptr 00000h]
fltFfs                  equ [word ptr 00002h]
ifldFormatFfs           equ [word ptr 00004h]
bchrFfs                 equ [word ptr 00006h]
fValidEndFfs            equ [word ptr 00008h]
xpFfs                   equ [word ptr 0000Ah]
ifldWrapFfs             equ [word ptr 0000Ch]
ifldErrorFfs            equ [word ptr 0000Eh]
cbFfsMin                equ 00010h

;
; struct FKP definition
;
rgfcFkp         equ     [dword ptr 00000h]
LO_rgfcFkp      equ     [word ptr 00000h]
HI_rgfcFkp      equ     [word ptr 00002h]
rgbFkp          equ     [00004h]
crunFkp 	equ	[byte ptr 001FFh]
cbFkpMin	equ	00200h

;
; struct FKPD definition
;
bFreeFirstFkpd		equ	[word ptr 00000h]
bFreeLimFkpd		equ	[word ptr 00002h]
pnFkpd			equ	[word ptr 00004h]
fcFirstFkpd		equ	[dword ptr 00006h]
LO_fcFirstFkpd		equ	[word ptr 00006h]
HI_fcFirstFkpd		equ	[word ptr 00008h]
fPlcIncompleteFkpd	equ	[word ptr 0000Ah]
chpFkpd 		equ	[0000Ch]
cbFkpdMin		equ	00018h

;
; struct FKPDT definition
;
bFreeFirstFkpdt 	equ	[word ptr 00000h]
bFreeLimFkpdt		equ	[word ptr 00002h]
pnFkpdt 		equ	[word ptr 00004h]
fcLimFkpdt		equ	[dword ptr 00006h]
LO_fcLimFkpdt		equ	[word ptr 00006h]
HI_fcLimFkpdt		equ	[word ptr 00008h]
cbFkpdtMin		equ	0000Ah

;
; struct FLCD definition
;
cpFirstFlcd		equ	[dword ptr 00000h]
LO_cpFirstFlcd		equ	[word ptr 00000h]
HI_cpFirstFlcd		equ	[word ptr 00002h]
dcpInstFlcd		equ	[dword ptr 00004h]
LO_dcpInstFlcd		equ	[word ptr 00004h]
HI_dcpInstFlcd		equ	[word ptr 00006h]
dcpResultFlcd		equ	[dword ptr 00008h]
LO_dcpResultFlcd	equ	[word ptr 00008h]
HI_dcpResultFlcd	equ	[word ptr 0000Ah]
fDirtyFlcd		equ	[word ptr 0000Ch]
fltFlcd 		equ	[word ptr 0000Eh]
bDataFlcd		equ	[word ptr 00010h]
ifldChBeginFlcd 	equ	[word ptr 00012h]
ifldChSeparateFlcd	equ	[word ptr 00014h]
ifldChEndFlcd		equ	[word ptr 00016h]
fDifferFlcd		equ	[byte ptr 00018h]
maskfDifferFlcd		equ	001h
ibitfDifferFlcd		equ	0
fResultDirtyFlcd	equ	[byte ptr 00018h]
maskfResultDirtyFlcd	equ	004h
ibitfResultDirtyFlcd	equ	2
fResultEditedFlcd	equ	[byte ptr 00018h]
maskfResultEditedFlcd	equ	008h
ibitfResultEditedFlcd	equ	3
fLockedFlcd		equ	[byte ptr 00018h]
maskfLockedFlcd		equ	010h
ibitfLockedFlcd		equ	4
fPrivateResultFlcd	equ	[byte ptr 00018h]
maskfPrivateResultFlcd	equ	020h
ibitfPrivateResultFlcd	equ	5
fNestedFlcd		equ	[byte ptr 00018h]
maskfNestedFlcd 	equ	040h
ibitfNestedFlcd 	equ	6
grpfFlcd		equ	[byte ptr 00018h]
cbFlcdMin		equ	00019h
;
; struct FLD definition
;
chFld			equ	[byte ptr 00000h]
maskchFld		equ	07Fh
ibitchFld		equ	0
fDirtyFld		equ	[byte ptr 00000h]
maskfDirtyFld		equ	080h
ibitfDirtyFld		equ	7
fltFld			equ	[byte ptr 00001h]
bDataFld		equ	[byte ptr 00001h]
fDifferFld		equ	[byte ptr 00001h]
maskfDifferFld		equ	001h
ibitfDifferFld		equ	0
fResultDirtyFld		equ	[byte ptr 00001h]
maskfResultDirtyFld	equ	004h
ibitfResultDirtyFld	equ	2
fResultEditedFld	equ	[byte ptr 00001h]
maskfResultEditedFld	equ	008h
ibitfResultEditedFld	equ	3
fLockedFld		equ	[byte ptr 00001h]
maskfLockedFld		equ	010h
ibitfLockedFld		equ	4
fPrivateResultFld	equ	[byte ptr 00001h]
maskfPrivateResultFld	equ	020h
ibitfPrivateResultFld	equ	5
fNestedFld		equ	[byte ptr 00001h]
maskfNestedFld		equ	040h
ibitfNestedFld		equ	6
grpfFld			equ	[byte ptr 00001h]
cbFldMin		equ	00002h


;
; struct FLI definition
;
cpMinFli		equ	[dword ptr 00000h]
LO_cpMinFli		equ	[word ptr 00000h]
HI_cpMinFli		equ	[word ptr 00002h]
cpMacFli		equ	[dword ptr 00004h]
LO_cpMacFli		equ	[word ptr 00004h]
HI_cpMacFli		equ	[word ptr 00006h]
docFli			equ	[word ptr 00008h]
wwFli			equ	[word ptr 0000Ah]
dxaFli			equ	[word ptr 0000Ch]
xpLeftFli		equ	[word ptr 0000Eh]
xpRightFli		equ	[word ptr 00010h]
dcpDependFli		equ	[word ptr 00012h]
fSplatsFli		equ	[byte ptr 00014h]
fSplatBreakFli		equ	[byte ptr 00014h]
maskFSplatBreakFli	equ	001h
fSplatDotsFli		equ	[byte ptr 00014h]
maskFSplatDotsFli	equ	002h
fSplatSubFli		equ	[byte ptr 00014h]
maskFSplatSubFli	equ	004h
fParaStoppedFli 	equ	[byte ptr 00014h]
maskFParaStoppedFli	equ	008h
fSplatColumnFli 	equ	[byte ptr 00014h]
maskFSplatColumnFli	equ	010h
fPropMarkFli		equ	[byte ptr 00014h]
maskFPropMarkFli	equ	020h
omkFli			equ	[byte ptr 00014h]
maskOmkFli		equ	0C0h
fParaAndSplatFli	equ	[byte ptr 00014h]
maskFParaAndSplatFli	equ	080h
fStopAtParaFli		equ	[byte ptr 00015h]
ichMacFli		equ	[byte ptr 00016h]
fSpecialJustFli 	equ	[byte ptr 00017h]
dypLineFli		equ	[word ptr 00018h]
dypAfterFli		equ	[word ptr 0001Ah]
dypBeforeFli		equ	[word ptr 0001Ch]
dypFontFli		equ	[word ptr 0001Eh]
dypBaseFli		equ	[word ptr 00020h]
xpSubFli		equ	[word ptr 00022h]
xpMarginRightFli	equ	[word ptr 00022h]
xpMarginLeftFli 	equ	[word ptr 00024h]
grpfBrcFli		equ	[word ptr 00026h]
fTopFli 		equ	[byte ptr 00026h]
ibitFTopFli		equ	0
maskFTopFli		equ	001h
fLeftFli		equ	[byte ptr 00026h]
ibitFLeftFli		equ	1
maskFLeftFli		equ	002h
fBottomFli		equ	[byte ptr 00026h]
ibitFBottomFli		equ	2
maskFBottomFli		equ	004h
fRightFli		equ	[byte ptr 00026h]
ibitFRightFli		equ	3
maskFRightFli		equ	008h
fTopEnableFli		equ	[byte ptr 00026h]
ibitFTopEnableFli	equ	4
maskFTopEnableFli	equ	010h
fBottomEnableFli	equ	[byte ptr 00026h]
ibitFBottomEnableFli	equ	5
maskFBottomEnableFli	equ	020h
fBetweenTopFli		equ	[byte ptr 00026h]
ibitFBetweenTopFli	equ	6
maskFBetweenTopFli	equ	040h
fBetweenBottomFli	equ	[byte ptr 00026h]
ibitFBetweenBottomFli	equ	7
maskFBetweenBottomFli	equ	080h
fBarTabsFli		equ	[byte ptr 00027h]
dypBrcTopFli		equ	[word ptr 00028h]
dypBrcBottomFli 	equ	[word ptr 0002Ah]
brcTopFli		equ	[word ptr 0002Ch]
brcLeftFli		equ	[word ptr 0002Eh]
brcBottomFli		equ	[word ptr 00030h]
brcRightFli		equ	[word ptr 00032h]
dxpExtraFli		equ	[word ptr 00034h]
ichSpace1Fli		equ	[byte ptr 00036h]
ichSpace2Fli		equ	[byte ptr 00037h]
ichSpace3Fli		equ	[byte ptr 00038h]
fPageViewFli		equ	[byte ptr 00039h]
fPrintFli		equ	[byte ptr 0003Ah]
fFormatAsPrintFli	equ	[byte ptr 0003Bh]
fOutlineFli		equ	[byte ptr 0003Ch]
fLayoutFli		equ	[byte ptr 0003Dh]
dxsInchFli		equ	[word ptr 0003Eh]
dxuInchFli		equ	[word ptr 00040h]
ichSpaceFli		equ	[byte ptr 00042h]
fVolatileFli		equ	[byte ptr 00043h]
fAdjustForVisiFli	equ	[byte ptr 00044h]
fErrorFli		equ	[byte ptr 00045h]
chBreakFli		equ	[word ptr 00046h]
grpfvisiFli		equ	[word ptr 00048h]
;bits in grpfvisi defined undef grpfvisi
dysInchFli		equ	[word ptr 0004Ah]
dyuInchFli		equ	[word ptr 0004Ch]
fRMarkFli		equ	[byte ptr 0004Eh]
maskFRMarkFli		equ	001h
fPictureFli		equ	[byte ptr 0004Eh]
maskFPictureFli 	equ	002h
fMetafileFli		equ	[byte ptr 0004Eh]
maskFMetafileFli	equ	004h
fGraphicsFli		equ	[byte ptr 0004Eh]
maskFGraphicsFli	equ	008h
dytLineFli		equ	[word ptr 00050h]
wREVIEW1Fli		equ	[word ptr 00052h]
rgchFli 		equ	[byte ptr 00054h]
rgdxpFli		equ	[word ptr 00154h]
cbFliMin		equ	00352h

;
; struct FLS definition
;
caFls			equ	[00000h]
wwFls			equ	[word ptr 0000Ah]
hplclnhFls		equ	[word ptr 0000Ch]
pheFls			equ	[0000Eh]
clMacFls		equ	[word ptr 00014h]
chBreakFls		equ	[byte ptr 00016h]
fVolatileFls		equ	[byte ptr 00017h]
fFirstColumnFls 	equ	[byte ptr 00018h]
fFirstParaFls		equ	[byte ptr 00019h]
fPageBreakFls		equ	[byte ptr 0001Ah]
fBreakLastFls		equ	[byte ptr 0001Bh]
fEndBreakFls		equ	[byte ptr 0001Ch]
fOutlineFls		equ	[byte ptr 0001Dh]
dxlFls			equ	[word ptr 0001Eh]
cbFlsMin		equ	00020h

;
; struct FMAL definition
;
fmtFmal         equ     [byte ptr 00000h]
maskFmtFmal     equ     0fh
ibitFmtFmal     equ     0
jcFmal          equ     [byte ptr 00000h]
maskJcFmal      equ     030h
ibitJcFmal      equ     4
fLineOrVarFmal  equ     [byte ptr 00000h]
maskFLineOrVarFmal      equ     040h
ibitFLineOrVarFmal      equ     6
fInLineFmal     equ     [byte ptr 00000h]
maskFInLineFmal equ     080h
ibitFInLineFmal equ     7
cParenFmal	equ	[byte ptr 00001h]
ichFmal         equ     [byte ptr 00002h]
dummyFmal	equ	[byte ptr 00003h]
dypAscentFmal	equ	[word ptr 00004h]
dytAscentFmal	equ	[word ptr 00006h]
dypDescentFmal	equ	[word ptr 00008h]
dytDescentFmal	equ	[word ptr 0000Ah]
dypFractFmal	equ	[word ptr 0000Ch]
xtFmal		equ	[word ptr 0000Eh]
xpFmal		equ	[word ptr 00010h]
bchrFmal	equ	[word ptr 00012h]
dytFractFmal	equ	[word ptr 00014h]
dummy1Fmal	equ	[word ptr 00016h]
dummy2Fmal	equ	[word ptr 00018h]
fmaFmal         equ     [00000h]
cpFmal		equ	[dword ptr 0001Ah]
LO_cpFmal	equ	[word ptr 0001Ah]
HI_cpFmal	equ	[word ptr 0001Ch]
docFmal 	equ	[word ptr 0001Eh]
fErrorFmal	equ	[word ptr 00020h]
fLiteralFmal	equ	[word ptr 00022h]
bchrChpFmal	equ	[word ptr 00024h]
pftiDxtFmal	equ	[word ptr 00026h]
cbFmalMin	equ	00028h

;
; struct FMA definition
;
fmtFma          equ     [byte ptr 00000h]
maskFmtFma      equ     0fh
ibitFmtFma      equ     0
jcFma           equ     [byte ptr 00000h]
maskJcFma       equ     030h
ibitJcFma       equ     4
fLineOrVarFma   equ     [byte ptr 00000h]
maskFLineOrVarFma       equ     040h
ibitFLineOrVarFma       equ     6
fInLineFma      equ     [byte ptr 00000h]
maskFInLineFma  equ     080h
ibitFInLineFma  equ     7
cParenFma	equ	[byte ptr 00001h]
ichFma          equ     [byte ptr 00002h]
dummyFma	equ	[byte ptr 00003h]
dypAscentFma	equ	[word ptr 00004h]
dytAscentFma	equ	[word ptr 00006h]
dypDescentFma	equ	[word ptr 00008h]
dytDescentFma	equ	[word ptr 0000Ah]
dypFractFma	equ	[word ptr 0000Ch]
xtFma		equ	[word ptr 0000Eh]
xpFma		equ	[word ptr 00010h]
bchrFma 	equ	[word ptr 00012h]
dytFractFma	equ	[word ptr 00014h]
lOptionsFma	equ	[dword ptr 00016h]
cbFmaMin	equ	0001Ah
; shiftCbFmh	equ	4

;
; struct FMTSS definition
;
tickFmtss       equ     [dword ptr 00000h]
LO_tickFmtss    equ     [word ptr 00000h]
HI_tickFmtss    equ     [word ptr 00002h]
cpRefFmtss      equ     [dword ptr 00004h]
LO_cpRefFmtss   equ     [word ptr 00004h]
HI_cpRefFmtss   equ     [word ptr 00006h]
pgnFmtss        equ     [word ptr 00008h]
lnnFmtss        equ     [word ptr 0000Ah]
docFmtss	equ	[word ptr 0000Ch]
fltFmtss	equ	[word ptr 0000Eh]
wFmtss		equ	[word ptr 00010h]
w2Fmtss         equ     [word ptr 00012h]
lFmtss		equ	[dword ptr 00014h]
LO_lFmtss	equ	[word ptr 00014h]
HI_lFmtss	equ	[word ptr 00016h]
dxpFmtss	equ	[word ptr 00018h]
dypFmtss	equ	[word ptr 0001Ah]
dxtFmtss	equ	[word ptr 0001Ch]
dytFmtss	equ	[word ptr 0001Eh]
dypDescentFmtss equ	[word ptr 00020h]
dytDescentFmtss equ	[word ptr 00022h]
cbFmtssMin	equ	00024h

;
;       struct FPC definition
;
hplcFpc			equ [word ptr 00000h]
pchFooFpc		equ [word ptr 00002h]
cbFooFpc		equ [word ptr 00004h]
bfooFpc			equ [word ptr 00006h]
ifdef DEBUG
ifooFpc			equ [word ptr 00008h]
cbFpcMin                equ 0000Ah
else
cbFpcMin                equ 00008h
endif

;
;	struct FRL definition
;
ylRejectFrl		equ [word ptr 00000h]
ylAcceptFrl		equ [word ptr 00002h]
ifndFrl 		equ [word ptr 00004h]
fNormalFrl		equ [byte ptr 00006h]
chSpareFrl		equ [byte ptr 00007h]
cbFrlMin		equ 00008h

;
; struct FTI definition
;
fcidFti                 equ     [dword ptr 00000h]
LO_fcidFti              equ     [word ptr 00000h]
HI_fcidFti		equ	[word ptr 00002h]
dxpOverhangFti          equ     [word ptr 00004h]
dypAscentFti            equ     [word ptr 00006h]
dypDescentFti           equ     [word ptr 00008h]
dypXtraAscentFti	equ     [word ptr 0000Ah]
fPrinterFti             equ     [byte ptr 0000Ch]
maskfPrinterFti 	equ	001h
shiftfPrinterFti	equ	0
fVisiBadFti             equ     [byte ptr 0000Ch]
maskfVisiBadFti 	equ	002h
shiftfVisiBadFti	equ	1
fFixedPitchFti		equ     [byte ptr 0000Ch]
maskfFixedPitchFti	equ	004h
fPrvwFti		equ	[byte ptr 0000Ch]
maskfPrvwFti		equ	008h
wREVIEW1Fti		equ     [word ptr 0000Eh]
dxpInchFti              equ     [word ptr 00010h]
dypInchFti              equ     [word ptr 00012h]
dxpBorderFti            equ     [word ptr 00014h]
dypBorderFti            equ     [word ptr 00016h]
pfceFti                 equ     [word ptr 00018h]
hfontFti		equ	[word ptr 0001Ah]
dxpExpandedFti		equ	[word ptr 0001Ch]
fTossedPrinterDCFti	equ	[byte ptr 0001Eh]
maskfTossedPrinterDCFti equ	001h
rgdxpFti		equ	[word ptr 0020h]
cbFtiMin                equ     00220h
cbFtiFceSame            equ     (offset wREVIEW1Fti)

;
; struct FVB definition
;
docFvb			equ	[word ptr 00000h]
cpFirstFvb		equ	[dword ptr 00002h]
LO_cpFirstFvb		equ	[word ptr 00002h]
HI_cpFirstFvb		equ	[word ptr 00004h]
cpLimFvb		equ	[dword ptr 00006h]
LO_cpLimFvb		equ	[word ptr 00006h]
HI_cpLimFvb		equ	[word ptr 00008h]
cchFvb			equ	[word ptr 0000Ah]
ccrFvb			equ	[word ptr 0000Ch]
cchMaxFvb		equ	[word ptr 0000Eh]
rgchFvb 		equ	[word ptr 00010h]
ccrMaxFvb		equ	[word ptr 00012h]
rgcrFvb 		equ	[word ptr 00014h]
fOverflowFvb		equ	[word ptr 00016h]
cbFvbMin		equ	00018h

;
; struct GRPFVISI
;
maskfvisiTabsGrpfvisi		equ	00001h
maskfvisiSpacesGrpfvisi 	equ	00002h
maskfvisiParaMarksGrpfvisi	equ	00004h
maskfvisiFtnRefMarksGrpfvisi	equ	00008h
maskfvisiCondHyphensGrpfvisi	equ	00010h
maskfvisiShowAllGrpfvisi	equ	00020h
maskfNoShowPicturesGrpfvisi	equ	00040h
maskfSeeHiddenGrpfvisi		equ	00080h
maskflmGrpfvisi 		equ	00300h
shftFlmGrpfvisi	    	    	equ	008h
; 1 spare bit
maskgrpfShowResultsGrpfvisi	equ	01800h
maskfDrawTableDrsGrpfvisi	equ	02000h
maskfDrawPageDrsGrpfvisi	equ	04000h
maskfForceFieldGrpfvisi 	equ	08000h

;
; struct HDT
;
ihddHdt 		equ	[word ptr 00000h]
ylHdt			equ	[word ptr 00002h]
dylHdt			equ	[word ptr 00004h]
hpllrHdt		equ	[word ptr 00006h]
cbHdtMin		equ	00008h

;
; struct ITR
;
chDecimalItr		equ	[byte ptr 00000h]
chThousandItr		equ	[byte ptr 00001h]
chListItr		equ	[byte ptr 00002h]
szCurrencyItr		equ	[byte ptr 00003h]
fCurSepBlankItr 	equ	[byte ptr 00007h]
fInvalidChSettingItr	equ	[byte ptr 00008h]
iDateItr		equ	[byte ptr 00009h]
iTimeItr		equ	[byte ptr 0000Ah]
iLDateItr		equ	[byte ptr 0000Bh]
chDateItr		equ	[byte ptr 0000Ch]
chTimeItr		equ	[byte ptr 0000Dh]
rgszMornEveItr		equ	[byte ptr 0000Eh]
fCurPostfixItr		equ	[byte ptr 00018h]
fLZeroItr		equ	[byte ptr 00019h]
iDigitsItr		equ	[byte ptr 0001Ah]
fMetricItr		equ	[byte ptr 0001Bh]
chPageNoItr		equ	[byte ptr 0001Ch]
chSectNoItr		equ	[byte ptr 0001Dh]
fScandanavianItr	equ	[byte ptr 0001Eh]
fFrenchItr		equ	[byte ptr 0001Fh]

;
; struct LBS
;
docLbs			equ	[word ptr 00000h]
cpLbs			equ	[dword ptr 00002h]
LO_cpLbs		equ	[word ptr 00002h]
HI_cpLbs		equ	[word ptr 00004h]
clLbs			equ	[word ptr 00006h]
ylLbs			equ	[word ptr 00008h]
pgnLbs			equ	[word ptr 0000Ah]
ilrCurLbs		equ	[word ptr 0000Ch]
hpllrLbs		equ	[word ptr 0000Eh]
ylColumnLbs		equ	[word ptr 00010h]
ylMaxBlockLbs		equ	[word ptr 00012h]
dylOverlapLbs		equ	[word ptr 00014h]
dylUsedColLbs		equ	[word ptr 00016h]
xlLbs			equ	[word ptr 00018h]
dxlColumnLbs		equ	[word ptr 0001Ah]
wwLbs			equ	[word ptr 0001Ch]
lnnLbs			equ	[word ptr 0001Eh]
ihdtLbs 		equ	[word ptr 00020h]
dcpDependLbs		equ	[word ptr 00022h]
fRightLbs		equ	[byte ptr 00024h]
fPgnRestartLbs		equ	[byte ptr 00025h]
fFacingPagesLbs 	equ	[byte ptr 00026h]
fOutlineLbs		equ	[byte ptr 00027h]
fSimpleLrLbs		equ	[byte ptr 00028h]
fPostScriptLbs		equ	[byte ptr 00029h]
lbcLbs			equ	[byte ptr 0002Ah]
bkcLbs			equ	[byte ptr 0002Bh]
ihdtTopLbs		equ	[byte ptr 0002Ch]
ihdtBottomLbs		equ	[byte ptr 0002Dh]
fEmptyPageLbs		equ	[byte ptr 0002Eh]
fCpOnPageLbs		equ	[byte ptr 0002Fh]
fContinueLbs		equ	[byte ptr 0002Ah]
fFirstColumnLbs 	equ	[byte ptr 00030h]
fFirstParaLbs		equ	[byte ptr 00031h]
fEndnotesLbs		equ	[byte ptr 00032h]
fNoBalanceLbs		equ	[byte ptr 00033h]
fAbsPresentLbs		equ	[byte ptr 00034h]
fOnLbsStackLbs		equ	[byte ptr 00035h]
ilrFirstChainLbs	equ	[word ptr 00036h]
ilrCurChainLbs		equ	[word ptr 00038h]
dylChainLbs		equ	[word ptr 0003Ah]
dylBaseChainLbs 	equ	[word ptr 0003Ch]
dylBelowTableLbs	equ	[word ptr 0003Eh]
fPrevTableLbs		equ	[byte ptr 00040h]
fBkRepagStopLbs 	equ	[byte ptr 00041h]
hpllnbcLbs		equ	[word ptr 00042h]
clTotalLbs		equ	[word ptr 00044h]
dyllTotalLbs		equ	[dword ptr 00046h]
LO_dyllTotalLbs 	equ	[word ptr 00046h]
HI_dyllTotalLbs 	equ	[word ptr 00048h]
ylMaxLrLbs		equ	[word ptr 0004Ah]
ylMaxColumnLbs		equ	[word ptr 0004Ch]
dylBeforeUsedLbs	equ	[word ptr 0004Eh]
cbLbsMin		equ	00050h

;
; struct LCB definition
;
caLcb		    equ     [00000h]
yaPpLcb 	    equ     [word ptr 0000Ah]
lnnLcb		    equ     [word ptr 0000Ch]
ichLcb		    equ     [word ptr 0000Eh]
cbLcbMin	    equ     00010h

;
; struct LNH definition
;
fSplatOnlyLnh	    equ     [byte ptr 00000h]
maskfSplatOnlyLnh   equ     001h
fDypBeforeAddedLnh  equ     [byte ptr 00000h]
maskfDypBeforeAddedLnh  equ 002h
chBreakLnh	    equ     [byte ptr 00001h]
yllLnh		    equ     [dword ptr 00002h]
LO_yllLnh	    equ     [word ptr 00002h]
HI_yllLnh	    equ     [word ptr 00004h]
cbLnhMin	    equ     00006h

;
; struct LOGFONT definition
;
lfHeightLogfont         equ     [word ptr 00000h]
lfWidthLogfont          equ     [word ptr 00002h]
lfEscapementLogfont     equ     [word ptr 00004h]
lfOrientationLogfont    equ     [word ptr 00006h]
lfWeightLogfont         equ     [word ptr 00008h]
lfItalicLogfont         equ     [byte ptr 0000Ah]
lfUnderlineLogfont      equ     [byte ptr 0000Bh]
lfStrikeOutLogfont      equ     [byte ptr 0000Ch]
lfCharSetLogfont        equ     [byte ptr 0000Dh]
lfOutPrecisionLogfont   equ     [byte ptr 0000Eh]
lfClipPrecisionLogfont  equ     [byte ptr 0000Fh]
lfQualityLogfont        equ     [byte ptr 00010h]
lfPitchAndFamilyLogfont equ     [byte ptr 00011h]
lfFaceNameLogfont       equ     [00012h]
cbLogfontMin            equ     00032h

;
; struct LR definition
;
drclLr			equ	[00000h]
xlLr			equ	[word ptr 00000h]
ylLr			equ	[word ptr 00002h]
dxlLr			equ	[word ptr 00004h]
dylLr			equ	[word ptr 00006h]
caLr			equ	[00008h]
cpLr			equ	[dword ptr 00008h]
LO_cpLr 		equ	[word ptr 00008h]
HI_cpLr 		equ	[word ptr 0000Ah]
cpLimLr 		equ	[dword ptr 0000Ch]
LO_cpLimLr		equ	[word ptr 0000Ch]
HI_cpLimLr		equ	[word ptr 0000Eh]
docLr			equ	[word ptr 00010h]
clFirstLr		equ	[word ptr 00012h]
clLimLr 		equ	[word ptr 00014h]
lnnLr			equ	[word ptr 00016h]
ihdtLr			equ	[word ptr 00018h]
fGraphicsLr		equ	[byte ptr 0001Ah]
maskfGraphicsLr 	equ	001h
fForceFirstRowLr	equ	[byte ptr 0001Ah]
maskfForceFirstRowLr	equ	002h
fColAfterAbsLr		equ	[byte ptr 0001Ah]
maskfColAfterAbsLr	equ	008h
fPageAfterAbsLr 	equ	[byte ptr 0001Ah]
maskfPageAfterAbsLr	equ	010h
fInlineLr		equ	[byte ptr 0001Ah]
maskfInlineLr		equ	020h
fConstrainLeftLr	equ	[byte ptr 0001Ah]
maskfConstrainLeftLr	equ	040h
fConstrainRightLr	equ	[byte ptr 0001Ah]
maskfConstrainRightLr	equ	080h
fConstrainTopLr 	equ	[byte ptr 0001Bh]
maskfConstrainTopLr	equ	001h
fChainLr		equ	[byte ptr 0001Bh]
maskfChainLr		equ	002h
fSpaceBeforeLr		equ	[byte ptr 0001Bh]
maskfSpaceBeforeLr	equ	004h
tSoftBottomLr		equ	[byte ptr 0001Bh]
maskTSoftBottomLr	equ	0F0h
lrsLr			equ	[byte ptr 0001Ch]
lrkLr			equ	[byte ptr 0001Dh]
ilrNextChainLr		equ	[word ptr 0001Eh]
cpMacCountedLr		equ	[dword ptr 00020h]
LO_cpMacCountedLr	equ	[word ptr 00020h]
HI_cpMacCountedLr	equ	[word ptr 00022h]
cbLrMin 		equ	00024h

;
; struct MDCD definition
;
hdcMdcd 		equ	[word ptr 00000h]
pbmiMdcd		equ	[word ptr 00002h]

;
; struct MERR definition
;
hrgwEmerg1Merr          equ     [word ptr 00000h]
hrgwEmerg2Merr          equ     [word ptr 00002h]
fDocFullMerr            equ     [word ptr 00004h]
fFnFullMerr             equ     [word ptr 00006h]
fWwFullMerr             equ     [word ptr 00008h]
fMwFullMerr             equ     [word ptr 0000Ah]
fSystemLockedMerr		equ	[byte ptr 0000Ch]
maskfSystemLockedMerr		equ	001h
fWordLockedMerr 		equ	[byte ptr 0000Ch]
maskfWordLockedMerr		equ	002h
fLockAlertMerr			equ	[byte ptr 0000Ch]
maskfLockAlertMerr		equ	004h
fScratchFileInitMerr		equ	[byte ptr 0000Ch]
maskfScratchFileInitMerr	equ	008h
fUndoCancelMerr 		equ	[byte ptr 0000Ch]
maskfUndoCancelMerr		equ	010h
fSaveFailMerr			equ	[byte ptr 0000Ch]
maskfSaveFailMerr		equ	040h
fPostponeScratchMerr		equ	[byte ptr 0000Ch]
maskfPostponeScratchMerr	equ	080h
fErrorAlertMerr 		equ	[byte ptr 0000Dh]
maskfErrorAlertMerr		equ	001h
fHadMemAlertMerr		equ	[byte ptr 0000Dh]
maskfHadMemAlertMerr		equ	002h
fInhibitMerr			equ	[byte ptr 0000Dh]
maskfInhibitMerr		equ	004h
fHadPrintAlertMerr		equ	[byte ptr 0000Dh]
maskfHadPrintAlertMerr		equ	008h
fNoHdrDrMerr			equ	[byte ptr 0000Dh]
maskfNoHdrDrMerr		equ	010h
fHadDispAlertMerr		equ	[byte ptr 0000Dh]
maskfHadDispAlertMerr		equ	020h
fSdmInitMerr			equ	[byte ptr 0000Dh]
maskfSdmInitMerr		equ	040h
fReclaimHprcsMerr		equ	[byte ptr 0000Dh]
maskfReclaimHprcsMerr		equ	080h
matMerr			equ     [word ptr 0000Eh]
fMemFailMerr		equ	[word ptr 0000Eh]
fDiskFailMerr           equ     [word ptr 00010h]
fDiskEmergMerr          equ     [byte ptr 00010h]
maskfDiskEmergMerr      equ     001h
fDiskAlertMerr		equ	[byte ptr 00010h]
maskfDiskAlertMerr	equ	002h
fDiskWriteErrMerr	equ	[byte ptr 00010h]
maskfDiskWriteErrMerr	equ	004h
fPrintEmergMerr         equ     [word ptr 00012h]
fDclFullMerr            equ     [word ptr 00014h]
hrgwEmerg3Merr		equ	[word ptr 00016h]
fWarnDocTooBigMerr	equ	[word ptr 00018h]
prmDontReclaimMerr	equ	[word ptr 0001Ah]

fKillRulRibMerr		equ	[byte ptr 0001Ch]
maskfKillRulRibMerr	equ	03h  ; 2 bits

fKillRulerMerr		equ	[byte ptr 0001Ch]
maskfKillRulerMerr	equ	01h
fKillRibbonMerr		equ	[byte ptr 0001Ch]
maskfKillRibbonMerr	equ	02h
fFmtFailedMerr  	equ	[word ptr 0001Ch]
maskfFmtFailedMerr	equ	04h
fNoDrsMerr		equ	[word ptr 0001Ch]
maskfNoDrsMerr	equ	08h
fSavePheFailedMerr	equ	[word ptr 0001Ch]
maskfSavePheFailedMerr	equ	010h
cbMerrMin		equ	0001Eh

;
; struct MWD definition
;
fSplitMwd               equ     [word ptr 00000h]
maskfSplitMwd		equ	001h
fActiveMwd              equ     [word ptr 00000h]
maskfActiveMwd		equ	002h
fAtnActiveMwd		equ	[word ptr 00000h]
maskfAtnActiveMwd	equ	004h
wwActiveMwd		equ	[byte ptr 00000h]
maskwwActiveMwd 	equ	0F8h
wwRulerMwd		equ	[byte ptr 00001h]
maskwwRulerMwd		equ	01Fh
wwUpperMwd		equ	[byte ptr 00002h]
wwLowerMwd		equ	[byte ptr 00003h]
rgwwMwd 		equ	[byte ptr 00002h]
docMwd			equ	[word ptr 00004h]
hwndRulerMwd		equ	[word ptr 00006h]
hwndMwd 		equ	[word ptr 00008h]
docMcrMwd		equ	[word ptr 0000Ah]
imcrMwd 		equ	[word ptr 0000Ch]
; 2 words for a union
fHorzScrollBarMwd	equ	[byte ptr 0000Eh]
maskfHorzScrollBarMwd	equ	001h
fVertScrollBarMwd	equ	[byte ptr 0000Eh]
maskfVertScrollBarMwd	equ	002h
lvlOutlineMwd		equ	[byte ptr 0000Eh]
masklvlOutlineMwd	equ	03Ch
hwndHScrollMwd		equ	[byte ptr 00010h]
hwndSplitBoxMwd 	equ	[byte ptr 00012h]
hwndSizeBoxMwd		equ	[byte ptr 00014h]
hwndSplitBarMwd 	equ	[byte ptr 00016h]
ypSplitMwd		equ	[byte ptr 00018h]
xpMwd			equ	[byte ptr 0001Ah]
ypmwd			equ	[byte ptr 0001Ch]
dxpMwd			equ	[byte ptr 0001Eh]
dypMwd			equ	[byte ptr 00020h]
wSpare2Mwd		equ	[byte ptr 00022h]
wSpare1Mwd		equ	[byte ptr 00024h]
cbMwdMin		equ	00026h

;
; struct PAD definition
;
fInTablePad 		equ	[byte ptr 00000h]
maskfInTablePad		equ	001h
fUnkPad 		equ	[byte ptr 00000h]
maskfUnkPad		equ	002h
fShowPad		equ	[byte ptr 00000h]
maskfShowPad		equ	004h
fBodyPad		equ	[byte ptr 00000h]
maskfBodyPad		equ	008h
lvlPad			equ	[byte ptr 00000h]
masklvlPad		equ	0F0h
stcPad			equ	[byte ptr 00001h]
cbPadMin		equ	00002h

;
; struct PAP definition
;
stcPap			equ	[byte ptr 00000h]
jcPap			equ	[byte ptr 00001h]
fSideBySidePap		equ	[byte ptr 00002h]
fKeepPap		equ	[byte ptr 00003h]
fKeepFollowPap		equ	[byte ptr 00004h]
fPageBreakBeforePap	equ	[byte ptr 00005h]
fBrLnAbovePap		equ	[byte ptr 00006h]
maskFBrLnAbovePap	equ	001h
shiftFBrLnAbovePap	equ	00h
fBrLnBelowPap		equ	[byte ptr 00006h]
maskFBrLnBelowPap	equ	002h
shiftFBrLnBelowPap	equ	01h
pcPap			equ	[byte ptr 00006h]
maskPcPap		equ	0F0h
shiftPcPap		equ	04h
pcVertPap		equ	[byte ptr 00006h]
maskPcVertPap		equ	030h
pcHorzPap		equ	[byte ptr 00006h]
maskPcHorzPap		equ	0C0h
brcpPap 		equ	[byte ptr 00007h]
brclPap 		equ	[byte ptr 00008h]
nfcSeqNumbPap		equ	[byte ptr 00009h]
nnSeqNumbPap		equ	[byte ptr 0000Ah]
fNoLnnPap		equ	[byte ptr 0000Bh]
dxaRightPap		equ	[word ptr 0000Ch]
dxaLeftPap		equ	[word ptr 0000Eh]
dxaLeft1Pap		equ	[word ptr 00010h]
dyaLinePap		equ	[word ptr 00012h]
dyaBeforePap		equ	[word ptr 00014h]
dyaAfterPap		equ	[word ptr 00016h]
phePap			equ	00018h
fInTablePap		equ	[byte ptr 0001Eh]
fTtpPap 		equ	[byte ptr 0001Fh]
ptapPap 		equ	[word ptr 00020h]
dxaAbsPap		equ	[word ptr 00022h]
dyaAbsPap		equ	[word ptr 00024h]
dxaWidthPap		equ	[word ptr 00026h]
brcTopPap		equ	[word ptr 00028h]
brcLeftPap		equ	[word ptr 0002Ah]
brcBottomPap		equ	[word ptr 0002Ch]
brcRightPap		equ	[word ptr 0002Eh]
brcBetweenPap		equ	[word ptr 00030h]
brcBarPap		equ	[word ptr 00032h]
dxaFromTextPap		equ	[word ptr 00034h]
wREVIEW1Pap	equ	[word ptr 00036h]
wREVIEW2Pap	equ	[word ptr 00038h]
itbdMacPap	equ	[word ptr 0003Ah]
rgdxaTabPap	equ	[word ptr 0003Ch]
rgtbdPap	equ	[byte ptr 000A0h]
cbPapMin	equ	000D2h
cbPAP		equ	(000D2h)
cwPAP		equ	(cbPAP/00002h)
cbPAPS		equ	(0001Eh)
cwPAPS		equ	(cbPAPS/00002h)
cbPAPBase	equ	(cbPAP - (itbdMax*3))
cwPAPBase	equ	(cbPAPBase/00002h)
cbPAPBaseScan	equ	(wREVIEW2Pap)

;
; struct PCD definition
;
fNoParaLastPcd          equ     [byte ptr 00000h]
maskFNoParaLastPcd      equ     001h
fPaphNilPcd             equ     [byte ptr 00000h]
maskFPaphNilPcd         equ     002h
fCopiedPcd              equ     [byte ptr 00000h]
maskFCopiedPcd          equ     004h
ifdef DEBUG
fNoParaLastValidPcd     equ     [byte ptr 00000h]
maskFNoParaLastValidPcd equ     080h
endif
fnPcd           equ     [byte ptr 0001h]
fcPcd           equ     [dword ptr 00002h]
LO_fcPcd        equ     [word ptr 00002h]
HI_fcPcd        equ     [word ptr 00004h]
prmPcd          equ     [word ptr 00006h]
cbPcdMin        equ     00008h
cbPCD           equ     (00008h)
cwPCD           equ     (00008h/00002h)

;
; struct PCVH definition
;
fVertPcvh		equ	[byte ptr 00000h]
maskFVertPcvh		equ	008h
pcVertPcvh		equ	[byte ptr 00000h]
maskPcVertPcvh		equ	030h
pcHorzPcvh		equ	[byte ptr 00000h]
maskPcHorzPcvh		equ	0C0h
opPcvh			equ	[byte ptr 00000h]
cbPcvhMin		equ	00002h

;
; struct PGD definition
;
fGhosePgd		equ	[byte ptr 00000h]
maskFGhostPgd		equ	060h
fContinuePgd		equ	[byte ptr 00000h]
maskFContinuePgd	equ	001h
fUnkPgd 		equ	[byte ptr 00000h]
maskFUnkPgd		equ	002h
fRightPgd		equ	[byte ptr 00000h]
maskFRightPgd		equ	008h
fPgnRestartPgd		equ	[byte ptr 00000h]
maskFPgnRestartPgd	equ	010h
fEmptyPagePgd		equ	[byte ptr 00000h]
maskFEmptyPagePgd	equ	020h
fAllFtnPgd		equ	[byte ptr 00000h]
maskFAllFtnPgd		equ	040h
bkcPgd			equ	[byte ptr 00001h]
lnnPgd			equ	[word ptr 00002h]
clPgd			equ	[word ptr 00004h]
pgnPgd			equ	[word ptr 00006h]
dcpDependPgd		equ	[word ptr 00008h]
cbPgdMin		equ	0000Ah

;
; struct PHE definition
;
fUnkPhe 		equ	[byte ptr 00000h]
maskFUnkPhe		equ	002h
fDiffLinesPhe		equ	[byte ptr 00000h]
maskFDiffLinesPhe	equ	004h
clMacPhe		equ	[byte ptr 00001h]
w0Phe			equ	[word ptr 00000h]
dxaColPhe		equ	[word ptr 00002h]
dylLinePhe		equ	[word ptr 00004h]
dyaLinePhe		equ	[word ptr 00004h]
dylHeightPhe		equ	[word ptr 00004h]
dyaHeightPhe		equ	[word ptr 00004h]
fStyleDirtyPhe		equ	[word ptr 00004h]
cbPheMin		equ	00006h

;
; struct PL definition
;
iMacPl		equ	[word ptr 00000h]
iMaxPl		equ	[word ptr 00002h]
cbPl		equ	[word ptr 00004h]
brgfooPl	equ	[word ptr 00006h]
fExternalPl	equ	[word ptr 00008h]
cbPlBase	equ	0000Ah

;
; struct PLC definition
;
iMacPlcSTR      equ     [word ptr 00000h]
iMaxPlc         equ     [word ptr 00002h]
cbPlc           equ     [word ptr 00004h]
icpAdjustPlc	equ	[word ptr 00006h]
dcpAdjustPlc	equ	[dword ptr 00008h]
LO_dcpAdjustPlc equ	[word ptr 00008h]
HI_dcpAdjustPlc equ	[word ptr 0000Ah]
icpHintPlc	equ	[word ptr 0000Ch]
fExternalPlc	equ	[byte ptr 0000Eh]
maskFExternalPlc	equ	001h
fExtBasePlc	equ	[byte ptr 0000Eh]
maskFExtBasePlc 	equ	002h
fMultPlc	equ	[byte ptr 0000Eh]
maskFMultPlc		equ	008h
rgcpPlc 	equ	[dword ptr 00010h]
LO_rgcpPlc	equ	[word ptr 00010h]
HI_rgcpPlc      equ     [word ptr 00012h]
hqplcePlc	equ	[dword ptr 00010h]
LO_hqplcePlc	equ     [word ptr 00010h]
HI_hqplcePlc	equ     [word ptr 00012h]
cbPLCBase	equ	00010h
cbPLCMin	equ	00014h

;
; struct PLCBTE definition
;
ibteMacPlcbte   equ     [word ptr 00000h]
ibteMaxPlcbte   equ     [word ptr 00002h]
cbPlcbte        equ     [word ptr 00004h]
cbPlcbteMin     equ     00006h

;
; struct PLCEDL definition
;
dlMacPlcedl	equ	[word ptr 00000h]
dlMaxPlcedl	equ	[word ptr 00002h]
cbPlcedl	equ	[word ptr 00004h]
icpAdjustPlcedl equ	[word ptr 00006h]
dcpAdjustPlcedl equ	[dword ptr 00008h]
icpHintPlcedl	equ	[word ptr 0000Ch]
fExternalPlcedl equ	[byte ptr 0000Eh]
maskFExternalPlcedl	equ	001h
fExtBasePlcedl	equ	[byte ptr 0000Eh]
maskFExtBasePlcedl	equ	002h
rgcpPlcedl	equ	[word ptr 00010h]
cbPlcedlMin	equ	00014h

;
; struct PLCPCD definition
;
ipcdMacPlcpcd   equ     [word ptr 00000h]
ipcdMaxPlcpcd   equ     [word ptr 00002h]
cbPlcpcd        equ     [word ptr 00004h]

;
; struct PLDR definition
;
idrMacPldr	equ	[word ptr 00000h]
idrMaxPldr	equ	[word ptr 00002h]
cbDrPldr	equ	[word ptr 00004h]
brgdrPldr	equ	[word ptr 00006h]
fExternalPldr	equ	[word ptr 00008h]
hpldrBackPldr	equ	[word ptr 0000Ah]
idrBackPldr	equ	[word ptr 0000Ch]
ptOriginPldr	equ	0000Eh
dylPldr 	equ	[word ptr 00012h]
rgdrPldr	equ	00014h
hqpldrePldr	equ	00014h
LO_hqpldrePldr	equ	00014h
HI_hqpldrePldr	equ	00016h

;
; struct PPR definition
;
ichPpr			equ	[word ptr 00000h]
cchPpr			equ	[word ptr 00002h]
nLastPpr		equ	[word ptr 00004h]
nIncrPpr		equ	[word ptr 00006h]
fAbortCheckPpr		equ     [word ptr 00008h]
hpprPrevPpr		equ	[word ptr 0000Ah]
hstPrevPpr		equ	[word ptr 0000Ch]
ifdef WIN23
xpPpr			equ	[word ptr 0000Eh]
cbPprMin		equ	00010h

else
cbPprMin		equ	0000Eh

endif

;
; struct PRC definition
;
fRefPrc         equ     [word ptr 00000h]
maskfRefPrc     equ     00001h
wChecksumPrc    equ     [word ptr 00000h]
maskwChecksumPrc    equ 0FFFEh
hprcNextPrc     equ     [word ptr 00002h]
bprlMacPrc      equ     [word ptr 00004h]
grpprlPrc       equ     [00006h]
cbPrcMin        equ     00006h
cbPRCBase       equ     6

;
; struct PREF definition
;
wSparePref              equ     [word ptr 00000h]
utPref			equ	[byte ptr 00002h]
maskutPref		equ	00fh
fPreLoadFilePref	equ	[byte ptr 00002h]
maskfPreLoadFilePref	equ	010h
fPreLoadWordPref	equ	[byte ptr 00002h]
maskfPreLoadWordPref	equ	020h
fRibbonPref		equ	[byte ptr 00002h]
maskfRibbonPref 	equ	040h
fStatLinePref		equ	[byte ptr 00002h]
maskfStatLinePref	equ	080h
fPromptSIPref		equ	[byte ptr 00003h]
maskFPromptSIPref	equ	001h
fBkgrndPagPref		equ	[byte ptr 00003h]
maskfBkgrndPagPref	equ	002h
fAutoDeletePref 	equ	[byte ptr 00003h]
maskfAutoDeletePref	equ	004h
fShortMenusPref 	equ	[byte ptr 00003h]
maskfShortMenusPref	equ	008h
fRulerPref		equ	[byte ptr 00003h]
maskfRulerPref		equ	010h
fHorzScrollBarPref	equ	[byte ptr 00003h]
maskfHorzScrollBarPref	equ	020h
fVertScrollBarPref	equ	[byte ptr 00003h]
maskfVertScrollBarPref	equ	040h
grpfvisiPref		equ     [word ptr 00004h]
wViewPrefsPref          equ     [word ptr 00004h]
;bits in grpfvisi defined under Grpfvisi
fPPSpellDetPref 	equ	[byte ptr 00006h]
maskfPPSpellDetPref	equ	001h
fPPHypPref		equ	[byte ptr 00006h]
maskfPPHypPref		equ	002h
fPPPaginatePref 	equ	[byte ptr 00006h]
maskfPPPaginatePref	equ	004h
fPPGlossaryExpPref	equ	[byte ptr 00006h]
maskfPPGlossaryExpPref	equ	008h
iASPref 		equ	[byte ptr 00006h]
maskiASPref		equ	030h
fDraftViewPref		equ	[byte ptr 00006h]
maskfDraftViewPref	equ	040h
fZoomAppPref		equ	[byte ptr 00006h]
maskfZoomAppPref	equ	080h
fZoomMwdPref		equ	[byte ptr 00007h]
maskfZoomMwdPref	equ	001h
fEllipPref		equ	[byte ptr 00007h]
maskfEllipPref		equ	002h
fShowFPref		equ	[byte ptr 00007h]
maskfShowFPref		equ	004h
dxaStyWndPref           equ     [word ptr 00008h]
stUsrInitlPref          equ     [0000Ah]
stUsrNamePref           equ     [00010h]
rgwSparePref		equ	[word ptr 0042h]
cbPrefMin               equ     0004Eh
cbPREF                  equ     (0004Eh)
cwPREF                  equ     (0004Eh/00002h)

;
; struct PRI definition
;
pftiPri                 equ     [word ptr 00000h]
hdcPri                  equ     [word ptr 00002h]
hszPrinterPri		equ	[word ptr 00004h]
hszPrPortPri		equ	[word ptr 00006h]
hszPrDriverPri		equ	[word ptr 00008h]
wmmPri			equ	[byte ptr 0000Ah]
fDCFreePri		equ	[byte ptr 0000Bh]
maskFDCFreePri		equ	01h
fPrErrPri		equ	[byte ptr 0000Bh]
maskFPrErrPri		equ	02h
fICPri			equ	[byte ptr 0000Bh]
maskFICPri		equ	04h
fGraphicsPri		equ	[byte ptr 0000Bh]
maskFGraphicsPri	equ	08h
fTextPri		equ	[byte ptr 0000Bh]
maskFTextPri		equ	10h
hbrTextPri		equ	[word ptr 00046h]
fDPRPri			equ [word ptr 00050h]
; more fields, but not currently used from native

;
; struct PRM definition
;
prmPrm                  equ     [word ptr 00000h]
fComplexPrm             equ     [word ptr 00000h]
maskfComplexPrm         equ     00001h
sprmPrm                 equ     [word ptr 00000h]

; WARNING *** masksprmPrm assumed this value in DoPrmSgc() in fetchn.asm
masksprmPrm             equ     000feh
valPrm                  equ     [byte ptr 00001h]
cfgrPrcPrm              equ     [word ptr 00000h]

; WARNING *** maskcfgrPrcPrm assumed this value in DoPrmSgc() in fetchn.asm
maskcfgrPrcPrm          equ     0fffeh
cbPrmMin                equ     00002h

;
; struct PT definition
;
xpPt		equ	[word ptr 00000h]
ypPt		equ	[word ptr 00002h]
xwPt		equ	[word ptr 00000h]
ywPt		equ	[word ptr 00002h]
cbPtMin 	equ	00004h
cwPT		equ	(00004h/00002h)

;
; struct RC definition
;
xpLeftRc	equ	[word ptr 00000h]
xeLeftRc	equ	[word ptr 00000h]
xwLeftRc	equ	[word ptr 00000h]
xlLeftRc	equ	[word ptr 00000h]
ypTopRc 	equ	[word ptr 00002h]
yeTopRc 	equ	[word ptr 00002h]
ywTopRc		equ	[word ptr 00002h]
ylTopRc 	equ	[word ptr 00002h]
xpRightRc	equ	[word ptr 00004h]
xeRightRc	equ	[word ptr 00004h]
xwRightRc	equ	[word ptr 00004h]
xlRightRc	equ	[word ptr 00004h]
ypBottomRc	equ	[word ptr 00006h]
yeBottomRc	equ	[word ptr 00006h]
ywBottomRc	equ	[word ptr 00006h]
ylBottomRc	equ	[word ptr 00006h]
ptTopLeftRc	equ	[dword ptr 00000h]
LO_ptTopLeftRc	equ	[word ptr 00000h]
HI_ptTopLeftRc	equ	[word ptr 00002h]
ptBottomRightRc	equ	[dword ptr 00004h]
LO_ptBottomRightRc	equ	[word ptr 00004h]
HI_ptBottomRightRc	equ	[word ptr 00006h]
cbRcMin	equ	00008h
cwRC		equ	(00008h/00002h)

;
; struct RF definition
;
fTablePropsNinchRf	equ	[byte ptr 00000h]
maskFTablePropsNinchRf	equ	001h
fForceParaRf		equ	[byte ptr 00000h]
maskFForceParaRf	equ	002h
fNormalPrevRf		equ	[byte ptr 00000h]
maskFNormalPrevRf	equ	004h
fWaitForDrawRf		equ	[byte ptr 00000h]
maskFWaitForDrawRf	equ	008h
fChangeKeysRf		equ	[byte ptr 00000h]
maskFChangeKeysRf	equ	010h
fHotZValidRf		equ	[byte ptr 00000h]
maskFHotZValidRf	equ	020h
fHyphHaveWordRf 	equ	[byte ptr 00000h]
maskFHyphHaveWordRf	equ	040h
fRanCBTRf		equ	[byte ptr 00000h]
maskFRanCBTRf		equ	080h

fPgvRf			equ	[byte ptr 00001h]
maskFPgvRf		equ	001h
fCbtInitRf		equ	[byte ptr 00001h]
maskFCbtInitRf		equ	006h
fRibbonCBTRf		equ	[byte ptr 00001h]
maskFRibbonCBTRf	equ	018h
fInExternalCallRf		equ	[byte ptr 00001h]
maskFInExternalCallRf	equ	020h
fPauseRecorderRf	equ	[byte ptr 00001h]
maskFPauseRecorderRf	equ	040h
fSpare1Rf		equ	[byte ptr 00001h]
maskFSpare1Rf		equ	080h

fSpare2Rf		equ	[byte ptr 00002h]
maskFSpare2Rf		equ	001h
fMwCreateRf		equ	[byte ptr 00002h]
maskFCreateRf		equ	002h
fHyphCapitalsRf 	equ	[byte ptr 00002h]
maskFHyphCapitalsRf	equ	004h
fSplIgnoreCapsRf	equ	[byte ptr 00002h]
maskFSplIgnoreCapsRf	equ	008h
udcDefaultRf		equ	[byte ptr 00002h]
maskUdcDefaultRf	equ	030h
fPreloadSelFontRf	equ	[byte ptr 00002h]
maskFPreloadSelFontRf	equ	040h
fInFormatLineRf 	equ	[byte ptr 00002h]
maskFInFormatLineRf	equ	080h

fInsPgNumRf		equ	[byte ptr 00003h]
maskFInsPgNumRf 	equ	001h
fMustStopCBTRf		equ	[byte ptr 00003h]
maskFMustStopCBTRf	equ	002h
fNotFirstDlgRf		equ	[byte ptr 00003h]
maskFNotFirstDlgRf	equ	004h
fCkPgMargRf		equ	[byte ptr 00003h]
maskFCkPgMargRf 	equ	008h
fVetoViewRefRf		equ	[byte ptr 00003h]
maskFVetoViewRefRf	equ	010h
fInDyadicRf		equ	[byte ptr 00003h]
maskFInDyadicRf 	equ	020h
fExtendedMemoryRf	equ	[byte ptr 00003h]
maskFExtendedMemoryRf	equ	040h
fInDisplayFliRf 	equ	[byte ptr 00003h]
maskFInDisplayFliRf	equ	080h

fInFetchCpRf		equ	[byte ptr 00004h]
maskFInFetchCpRf	equ	080h

;
; struct RIBL definition
;
fInBodyRibl		equ	[byte ptr 00000h]
maskFInBodyRibl 	equ	001h
fCancelRibl		equ	[byte ptr 00000h]
maskFCancelRibl 	equ	002h
fNoColorRibl		equ	[byte ptr 00000h]
maskFNoColorRibl	equ	004h
fXeTcQOpenRibl		equ	[byte ptr 00000h]
maskFXeTcQOpenRibl	equ	008h
fSideBySideRibl 	equ	[byte ptr 00000h]
maskFSideBySideRibl	equ	010h
fHdrRibl		equ	[byte ptr 00000h]
maskFHdrRibl		equ	020h
fFtrRibl		equ	[byte ptr 00000h]
maskFFtrRibl		equ	040h
fFldImportRibl		equ	[byte ptr 00001h]
maskFFldImportRibl	equ	010h
fRevisionsRibl		equ	[byte ptr 00001h]
maskFRevisionsRibl	equ	020h
fDisregardPicRibl	equ	[byte ptr 00001h]
maskFDisregardPicRibl	equ	040h
fHiNibbleRibl		equ	[byte ptr 00001h]
maskFHiNibbleRibl	equ	080h
risRibl 		equ	[byte ptr 00002h]
iRibl			equ	[byte ptr 00003h]
bRibl			equ	[byte ptr 00004h]
bPicRibl		equ	[byte ptr 00005h]
bchSeqLimRibl		equ	[word ptr 00006h]
bchNumLimRibl		equ	[word ptr 00008h]
bchDataRibl		equ	[word ptr 0000Ah]
fStcNextOKRibl		equ	[byte ptr 0000Ch]
fNextIsDestRibl 	equ	[byte ptr 0000Dh]
iGridRibl		equ	[word ptr 0000Eh]
mtsRibl 		equ	[word ptr 00010h]
chsRibl 		equ	[word ptr 00012h]
fmcRibl 		equ	[word ptr 00014h]
deffRibl		equ	[word ptr 00016h]
pgnStartRibl		equ	[word ptr 00018h]
lineStartRibl		equ	[word ptr 0001Ah]
vernRibl		equ	[word ptr 0001Ch]
estcpRibl		equ	[0001Eh]
fFonttblDefRibl 	equ	[byte ptr 00020h]
cRedRibl		equ	[byte ptr 00021h]
cGreenRibl		equ	[byte ptr 00022h]
cBlueRibl		equ	[byte ptr 00023h]
lcbRibl 		equ	[dword ptr 00024h]
LO_lcbRibl		equ	[word ptr 00024h]
HI_lcbRibl		equ	[word ptr 00026h]
dxaPgnRibl		equ	[word ptr 00028h]
dyaPgnRibl		equ	[word ptr 0002Ah]
dyaHdrBottomRibl	equ	[word ptr 0002Ch]
dxaRightRibl		equ	[word ptr 0002Eh]
dyaBottomRibl		equ	[word ptr 00030h]
rdsRibl 		equ	[word ptr 00032h]
docRibl 		equ	[word ptr 00034h]
ihdtRibl		equ	[word ptr 00036h]
cpRdsFirstRibl		equ	[dword ptr 00038h]
LO_cpRdsFirstRibl	equ	[word ptr 00038h]
HI_cpRdsFirstRibl	equ	[word ptr 0003Ah]
iNibbleRibl		equ	[word ptr 0003Ch]
iTypePicRibl		equ	[word ptr 0003Eh]
stUsrInitlRibl		equ	[byte ptr 00040h]
hrgbStackRibl		equ	[word ptr 00046h]
bStackLimRibl		equ	[word ptr 00048h]
rhcRibl 		equ	[word ptr 0004Ah]
fcPicFirstRibl		equ	[dword ptr 0004Ch]
LO_fcPicFirstRibl	equ	[word ptr 0004Ch]
HI_fcPicFirstRibl	equ	[word ptr 0004Eh]
fWindowMessageRibl	equ	[byte ptr 00050h]
fDopPropsRecordedRibl	equ	[byte ptr 00051h]
hplftcmRibl		equ	[word ptr 00052h]
fPermOKRibl		equ	[word ptr 00054h]
cbCharInfoRibl		equ	[word ptr 00056h]
cbParaInfoRibl		equ	[word ptr 00058h]
cbSectInfoRibl		equ	[word ptr 0005Ah]
cbTableInfoRibl 	equ	[word ptr 0005Ch]
fParaBrcRibl		equ	[word ptr 0005Eh]
itcRibl 		equ	[word ptr 00060h]
ibrcRibl		equ	[word ptr 00062h]
tbdRibl 		equ	[00064h]
hrcpfldRibl		equ	[word ptr 00066h]
hsttbGridRibl		equ	[word ptr 00068h]
hrcpxextRibl		equ	[word ptr 0006Ah]
ibSeaRibl		equ	[word ptr 0006Ch]
rgbSeaRibl		equ	[byte ptr 0006Eh]
rgchRibl		equ	[byte ptr 00074h]
papRibl 		equ	[00174h]
rgbParaRibl		equ	[byte ptr 00174h]
chpRibl 		equ	[00246h]
rgbCharRibl		equ	[byte ptr 00246h]
sepRibl 		equ	[00252h]
rgbSectRibl		equ	[byte ptr 00252h]
tapRibl 		equ	[00270h]
rgbTableRibl		equ	[byte ptr 00270h]
rgbStcPermutationRibl	equ	[byte ptr 00408h]
hstGridRibl		equ	[word ptr 00508h]
icoMacRibl		equ	[word ptr 0050Ah]
mpicoOldicoNew		equ	[word ptr 0050Ch]
picRibl 		equ	[00570h]

;
; struct RRI definition
;
cpLowRptRri	equ	[dword ptr 00000h]
LO_cpLowRptRri	equ	[word ptr 00000h]
HI_cpLowRptRri	equ	[word ptr 00002h]
cpHighRptRri	equ	[dword ptr 00004h]
LO_cpHighRptRri	equ	[word ptr 00004h]
HI_cpHighRptRri	equ	[word ptr 00006h]
cpLimWrapRri	equ	[dword ptr 00008h]
LO_cpLimWrapRri	equ	[word ptr 00008h]
HI_cpLimWrapRri	equ	[word ptr 0000Ah]
cbRriMin	equ	0000Ch

;
; struct RULSS definition
;
caRulerSprmRulss    equ     [00000h]
cbGrpprlRulss	    equ     [word ptr 0000Ah]
hgrpprlRulss	    equ     [word ptr 0000Ch]
hrsdRulss	    equ     [word ptr 0000Eh]
cbRulssMin	    equ     00010h

;
; struct SAB definition
;
docStshSab              equ     [word ptr 00000h]
fExtSab 		equ	[byte ptr 00002h]
maskfExtSab		equ	0001h
fPictSab                equ     [byte ptr 00002h]
maskfPictSab		equ	0002h
fBlockSab               equ     [byte ptr 00002h]
maskfBlockSab		equ	0004h
fFormattedSab           equ     [byte ptr 00002h]
maskfFormattedSab	equ	0008h
fTableSab		equ	[byte ptr 00002h]
maskfTableSab		equ	0010h
fExtEqualSab            equ     [byte ptr 00002h]
maskfExtEqualSab	equ	0020h
fOwnClipboardSab        equ     [byte ptr 00002h]
maskfOwnClipboardSab	equ	0040h
fDontDestroyClipSab     equ     [byte ptr 00002h]
maskfDontDestroyClipSab equ	0080h
fMayHavePicSab		equ	[byte ptr 00003h]
maskfMayHavePicSab	equ	0001h
sabtSab 		equ	[byte ptr 00002h]
maskSabtSab		equ	001Fh
caCopySab               equ     [00004h]
cpFirstSab              equ     [dword ptr 00004h]
LO_cpFirstSab           equ     [word ptr 00004h]
HI_cpFirstSab           equ     [word ptr 00006h]
cpLimSab                equ     [dword ptr 00008h]
LO_cpLimSab             equ     [word ptr 00008h]
HI_cpLimSab             equ     [word ptr 0000Ah]
docSab                  equ     [word ptr 0000Ch]
cbSabMin                equ     0000Eh
cwSAB                   equ     0000Eh/00002h

;
; struct SCI definition
;
fMonochromeSci  equ     [word ptr 00000h]
fInvertMonochromeSci    equ     [word ptr 00002h]
rgbBkgrndSci    equ     [dword ptr 00004h]
LO_rgbBkgrndSci equ     [word ptr 00004h]
HI_rgbBkgrndSci equ     [word ptr 00006h]
rgbTextSci      equ     [dword ptr 00008h]
LO_rgbTextSci   equ     [word ptr 00008h]
HI_rgbTextSci   equ     [word ptr 0000Ah]
rgbBorderSci    equ     [dword ptr 0000Ch]
LO_rgbBorderSci equ     [word ptr 0000Ch]
HI_rgbBorderSci equ     [word ptr 0000Eh]
rgbDesktopSci   equ     [dword ptr 00010h]
LO_rgbDesktopSci        equ     [word ptr 00010h]
HI_rgbDesktopSci        equ     [word ptr 00012h]
hbrBkgrndSci    equ     [word ptr 00014h]
hbrTextSci      equ     [word ptr 00016h]
hbrDesktopSci   equ     [word ptr 00018h]
hbrBorderSci    equ     [word ptr 0001Ah]
hpenSci         equ     [word ptr 0001Ch]
hpenBorderSci   equ     [word ptr 0001Eh]
ropEraseSci     equ     [dword ptr 00020h]
LO_ropEraseSci  equ     [word ptr 00020h]
HI_ropEraseSci  equ     [word ptr 00022h]
mdcdScratchSci  equ     [dword ptr 00024h]
LO_mdcdScratchSci       equ     [word ptr 00024h]
HI_mdcdScratchSci       equ     [word ptr 00026h]
hdcScratchSci   equ     [word ptr 00024h]
pbmiScratchSci  equ     [word ptr 00026h]
mdcdBmpSci      equ     [dword ptr 00028h]
LO_mdcdBmpSci   equ     [word ptr 00028h]
HI_mdcdBmpSci   equ     [word ptr 0002Ah]
dcibMemDCSci    equ     [0002Ch]
dcibScreenSci   equ     [00034h]
pftiSci         equ     [word ptr 0003Ch]
dxpScreenSci    equ     [word ptr 0003Eh]
dypScreenSci    equ     [word ptr 00040h]
dxpScrlBarSci   equ     [word ptr 00042h]
dypScrlBarSci   equ     [word ptr 00044h]
dxpBorderSci    equ     [word ptr 00046h]
dypBorderSci    equ     [word ptr 00048h]
dypSplitBoxSci  equ     [word ptr 0004Ah]
dypSplitBarSci  equ     [word ptr 0004Ch]
dypRibbonSci	equ     [word ptr 0004Eh]
dypStatLineSci  equ     [word ptr 00050h]
dypRulerSci     equ     [word ptr 00052h]
dxpTmWidthSci   equ     [word ptr 00054h]
dypTmHeightSci  equ     [word ptr 00056h]
dypScrollArrow	equ	[word ptr 00058h]
wSpare1Sci      equ     [word ptr 0005Ah]
xpScrollMaxSci  equ     [word ptr 0005Ch]
dypComboSmallSci        equ     [word ptr 0005Eh]
dxpComboDeltaSci        equ     [word ptr 00060h]
xaLeftMinSci    equ     [word ptr 00062h]
xaRightMaxSci   equ     [word ptr 00064h]
dypIconBarSci   equ     [word ptr 00066h]
dxmmScreen	equ	[word ptr 00068h]
dymmScreen	equ	[word ptr 0006Ah]
wSpare3Sci      equ     [word ptr 0006Ch]
wSpare4Sci      equ     [word ptr 0006Eh]
wSpare5Sci      equ     [word ptr 00070h]
dxpMinScrollSci equ     [word ptr 00072h]
dxwSelBarSci    equ     [word ptr 00074h]
xpLeftMinSci    equ     [word ptr 00076h]
xpRightMaxSci   equ     [word ptr 00078h]
dysMinAveLineSci        equ     [word ptr 0007Ah]
dysMacAveLineSci        equ     [word ptr 0007Ch]
dypMinWwInitSci equ     [word ptr 0007Eh]
ypSubSuperSci   equ     [word ptr 00080h]
dypWwMinSci     equ     [word ptr 00082h]
dypPastPage	equ	[word ptr 00084h]
dypPageReveal	equ	[word ptr 00086h]
cbSciMin        equ     00088h

;
; struct SEA definition
;
rgchSea 	    equ     [byte ptr 00000h]
cbSeaMin	    equ     00006h

;
; struct SEBL definition
;
pgrpprlEarlierSebl  equ     [word ptr 00000h]
cbEarlierSebl	    equ     [word ptr 00002h]
pgrpprlMergeSebl    equ     [word ptr 00004h]
cbMergeSebl	    equ     [word ptr 00006h]
cbMergeMaxSebl	    equ     [word ptr 00008h]
pgrpprlLaterSebl    equ     [word ptr 0000Ah]
cbLaterSebl	    equ     [word ptr 0000Ch]
cbSeblMin	    equ     0000Eh

;
; struct SEL definition
;
fRightwardSel		equ	[byte ptr 00000h]
maskfRightwardSel       equ     001h
fSelAtParaSel		equ	[byte ptr 00000h]
maskfSelAtParaSel	equ	002h
fWithinCellSel		equ	[byte ptr 00000h]
maskfWIthinCellSel	equ	004h
fTableAnchorSel 	equ	[byte ptr 00000h]
maskfTableAnchorSel	equ	008h
fColumnSel		equ	[byte ptr 00001h]
maskfColumnSel		equ	004h
fTableSel		equ	[byte ptr 00001h]
maskfTableSel		equ	008h
fGraphicsSel		equ	[byte ptr 00001h]
maskfGraphicsSel	equ	010h
fBlockSel		equ	[byte ptr 00001h]
maskfBlockSel		equ	020h
fNilSel 		equ	[byte ptr 00001h]
maskfNilSel		equ	040h
fInsSel 		equ	[byte ptr 00001h]
maskFInsSel		equ	080h
skSel			equ	[byte ptr 00001h]
maskSkSel		equ	0FCh
ibitSkSel		equ	2
fForwardSel             equ     [byte ptr 00002h]
fInsEndSel              equ     [byte ptr 00003h]
cpFirstSel              equ     [dword ptr 00004h]
LO_cpFirstSel           equ     [word ptr 00004h]
HI_cpFirstSel           equ     [word ptr 00006h]
cpLimSel                equ     [dword ptr 00008h]
LO_cpLimSel             equ     [word ptr 00008h]
HI_cpLimSel             equ     [word ptr 0000Ah]
cpLastSel               equ     [dword ptr 00008h]
LO_cpLastSel            equ     [word ptr 00008h]
HI_cpLastSel            equ     [word ptr 0000Ah]
docSel                  equ     [word ptr 0000Ch]
caSel                   equ     [00004h]
xpFirstSel              equ     [word ptr 0000Eh]
xpLimSel                equ     [word ptr 00010h]
itcFirstSel		equ	[word ptr 0000Eh]
itcLimSel		equ	[word ptr 00010h]
cpAnchorSel		equ	[dword ptr 00012h]
LO_cpAnchorSel		equ	[word ptr 00012h]
HI_cpAnchorSel		equ	[word ptr 00014h]
stySel			equ	[word ptr 00016h]
itcOppositeAnchorSel	equ	[word ptr 00018h]
cpAnchorShrinkSel	equ  [dword ptr 0001Ah]
LO_cpAnchorShrinkSel	equ  [word ptr 0001Ah]
HI_cpAnchorShrinkSel	equ  [word ptr 0001Ch]
cbSelsMin	equ	0001Eh
; end of SELS
wwSel		equ	[word ptr 0001Eh]
paSel		equ	[word ptr 00020h]
cpFirstLineSel	equ	[dword ptr 00022h]
LO_cpFirstLineSel	equ	[word ptr 00022h]
HI_cpFirstLineSel	equ	[word ptr 00024h]
fHiddenSel	equ	[word ptr 00026h]
fOnSel		equ	[word ptr 00028h]
xwSel		equ	[word ptr 0002Ah]
ywSel		equ	[word ptr 0002Ch]
dypSel		equ	[word ptr 0002Eh]
tickOldSel	equ	[dword ptr 00030h]
LO_tickOldSel	equ	[word ptr 00030h]
HI_tickOldSel	equ	[word ptr 00032h]
fUpdateChpSel		equ	[byte ptr 00034h]
maskfUpdateChpSel       equ     001h
fUpdateChpGraySel	equ	[byte ptr 00034h]
maskfUpdateChpGraySel   equ     002h
fUpdatePapSel		equ	[byte ptr 00034h]
maskfUpdatePapSel       equ     004h
fUpdateRibbonSel	equ	[byte ptr 00034h]
maskfUpdateRibbonSel    equ     008h
fUpdateRulerSel 	equ	[byte ptr 00034h]
maskfUpdateRulerSel     equ     010h
fUpdateStatLineSel	equ	[byte ptr 00034h]
maskfUpdateStatLineSel  equ     020h
fUpdateStatLine2Sel	equ	[byte ptr 00034h]
maskfUpdateStatLine2Sel equ     040h
rcwClipSel	equ	[00036h]
chpSel		equ	[0003Eh]
cbSelMin	equ	0004Ah
cwSEL		equ	(004Ah/00002h)

;
; struct SEP definition
;
bkcSep			equ	[byte ptr 00000h]
fTitlePageSep		equ	[byte ptr 00001h]
ccolM1Sep		equ	[word ptr 00002h]
dxaColumnsSep		equ	[word ptr 00004h]
fAutoPgnSep		equ	[byte ptr 00006h]
nfcPgnSep		equ	[byte ptr 00007h]
pgnStartSep		equ	[word ptr 00008h]
wREVIEWSep		equ	[word ptr 0000Ah]
fPgnRestartSep		equ	[byte ptr 0000Ch]
fEndnoteSep		equ	[byte ptr 0000Dh]
lncSep			equ	[byte ptr 0000Eh]
grpfIhdtSep		equ	[byte ptr 0000Fh]
nLnnModSep		equ	[word ptr 00010h]
dxaLnnSep		equ	[word ptr 00012h]
dyaHdrTopSep		equ	[word ptr 00014h]
dyaHdrBottomSep 	equ	[word ptr 00016h]
fLBetweenSep		equ	[byte ptr 00018h]
vjcSep			equ	[byte ptr 00019h]
lnnMinSep		equ	[word ptr 0001Ah]
dxaColumnWidthSep       equ     [word ptr 0001Ch]
cbSepMin		equ	0001Eh
cbSEP			equ	(0001Eh)
cwSEP			equ	(cbSEP/00002h)

;
; struct SIAP definition
; WARNING *** SIAP structure assumed this way in N_ApplyPrlSgc() in fetchn.asm
hpsSizeSiap     equ     [byte ptr 00000h]
cIncSiap        equ     [byte ptr 00001h]
maskcIncSiap    equ     0007fh
fAdjSiap        equ     [byte ptr 00001h]
maskfAdjSiap	equ	00080h
hpsPosSiap      equ     [byte ptr 00002h]
cbSiapMin       equ     00003h
cbSIAP          equ     (00003h)

;
; struct SPX definition
;
rgsprmSpx	equ	[byte ptr 00000h]
cbSpxMin	equ	00002h

;
; struct STSH definition
;
cstcStdStsh     equ     [word ptr 00000h]
hsttbNameStsh   equ     [word ptr 00002h]
hsttbChpxStsh   equ     [word ptr 00004h]
hsttbPapxStsh   equ     [word ptr 00006h]
hplestcpStsh    equ     [word ptr 00008h]
cbStshMin       equ     0000Ah
cbSTSH          equ     (0000Ah)

;
; struct STTB definition
;
bMaxSttb	equ	[word ptr 00000h]
ibstMacSttb	equ	[word ptr 00002h]
ibstMaxSttb	equ	[word ptr 00004h]
cbExtraSttb	equ	[word ptr 00006h]
maskcbExtraSttb equ     03FFFh
fExternalSttb	equ	[byte ptr 00007h]
maskfExternalSttb equ	040h
fStyleRulesSttb equ	[byte ptr 00007h]
maskfStyleRulesSttb equ 080h
rgbstSttb	equ	[word ptr 00008h]
hqrgbstSttb	equ	[dword ptr 00008h]
OFF_hqrgbstSttb	equ	[word ptr 00008h]
SB_hqrgbstSttb	equ	[word ptr 0000Ah]
cbSttbMin	equ	0000Ch

;
; struct TAP definition
;
jcTap		    equ     [word ptr 00000h]
dxaGapHalfTap	    equ     [word ptr 00002h]
dyaRowHeightTap     equ     [word ptr 00004h]
grpfTap		    equ     [word ptr 00006h]
fCaFullTap	    equ     [byte ptr 00006h]
maskFCaFullTap	    equ     001h
fFirstRowTap	    equ     [byte ptr 00006h]
maskFFirstRowTap    equ     002h
fLastRowTap	    equ     [byte ptr 00006h]
maskFLastRowTap     equ     004h
fOutlineTap	    equ     [byte ptr 00006h]
maskFOutlineTap     equ     008h
itcMacTap	    equ     [word ptr 00008h]
dxaAdjustTap	    equ     [word ptr 0000Ah]
	errnz	<itcMax - 32>
rgdxaCenterTap	    equ     [word ptr 000Ch]
rgtcTap 	    equ     0004Eh
cbTapMin	    equ     0018Eh

;
; struct TBD definition
;
jcTbd           equ     [byte ptr 00000h]
maskJcTbd	equ	007h
tlcTbd          equ     [byte ptr 00000h]
maskTlcTbd      equ     038h
ibitTlcTbd      equ     3
cbTbdMin        equ     00001h

;
; struct TC definition
;
fFirstMergedTc	    equ     [byte ptr 00000h]
maskFFirstMergedTc  equ     001h
fMergedTc	    equ     [byte ptr 00000h]
maskFMergedTc	    equ     002h
brcTopTc	    equ     [word ptr 00002h]
brcLeftTc	    equ     [word ptr 00004h]
brcBottomTc	    equ     [word ptr 00006h]
brcRightTc	    equ     [word ptr 00008h]
rgbrcTc 	    equ     [word ptr 00002h]
cbTcMin 	    equ     0000Ah

;
; struct TCC definition
;
caTcc			equ	00000h
cpFirstTcc		equ	[dword ptr 00000h]
LO_cpFirstTcc		equ	[word ptr 00000h]
HI_cpFirstTcc		equ	[word ptr 00002h]
cpLimTcc		equ	[dword ptr 00004h]
LO_cpLimTcc		equ	[word ptr 00004h]
HI_cpLimTcc		equ	[word ptr 00006h]
docTcc			equ	[word ptr 00008h]
itcTcc			equ	[word ptr 0000Ah]
fFirstRowTcc		equ	[byte ptr 0000Ch]
maskfFirstRowTcc	equ	001h
fLastRowTcc		equ	[byte ptr 0000Ch]
maskfLastRowTcc 	equ	002h
fxpValidTcc		equ	[byte ptr 0000Ch]
maskfXpValidTcc 	equ	004h
fDylValidTcc		equ	[byte ptr 0000Ch]
maskfDylValidTcc	equ	008h
fOutlineTcc		equ	[byte ptr 0000Ch]
maskfOutlineTcc 	equ	010h
fEasyBordersTcc 	equ	[byte ptr 0000Ch]
maskfEasyBordersTcc	equ	020h
tcxTcc			equ	0000Eh
xpCellLeftTcc		equ	[word ptr 0000Eh]
xpDrLeftTcc		equ	[word ptr 00010h]
xpDrRightTcc		equ	[word ptr 00012h]
xpCellRightTcc		equ	[word ptr 00014h]
dxpOutLeftTcc		equ	[word ptr 00016h]
dxpOutRightTcc		equ	[word ptr 00018h]
brcLeftTcc		equ	[word ptr 0001Ah]
brcTopTcc		equ	[word ptr 0001Ch]
brcBottomTcc		equ	[word ptr 0001Eh]
brcRightTcc		equ	[word ptr 00020h]
dylAboveTcc		equ	[word ptr 00022h]
dylBelowTcc		equ	[word ptr 00024h]
rgbrcEasyTcc		equ	00026h
dxpInchTcc		equ	[word ptr 00030h]
wSpare2Tcc		equ	[word ptr 00032h]
caTapTcc		equ	00034h
caTapPrevTcc		equ	0003Eh
tapPrevTcc		equ	00048h
cbTccMin		equ	001E0h

;
; struct TCX definition
;
xpCellLeftTcx	equ	[word ptr 00000h]
xpDrLeftTcx	equ	[word ptr 00002h]
xpDrRightTcx	equ	[word ptr 00004h]
xpCellRightTcx	equ	[word ptr 00006h]
dxpOutLeftTcx	equ	[word ptr 00008h]
dxpOutRightTcx	equ	[word ptr 0000Ah]
rgbrcTcx	equ	[word ptr 0000Ch]
brcLeftTcx	equ	[word ptr 0000Ch]
brcTopTcx	equ	[word ptr 0000Eh]
brcBottomTcx	equ	[word ptr 00010h]
brcRightTcx	equ	[word ptr 00012h]
cbTcxMin	equ	00014h

;
; struct TCXC definition
;
fValidTcxc	equ	[word ptr 00000h]
itcNextTcxc	equ	[word ptr 00002h]
tcxTcxc		equ	[word ptr 00004h]
cbTcxcMin	equ	00018h

;
; struct TCXS definition
;
caTcxs		equ	[00000h]
grpfTapTcxs	equ	[word ptr 0000Ah]
dxpInchTcxs	equ	[word ptr 0000Ch]
rgtcxcTcxs	equ	[0000Eh]

;
; struct TN definition
;
tntTn		equ	[word ptr 00000h]
hTn		equ	[word ptr 00002h]
cTn		equ	[word ptr 00004h]
cbTnMin 	equ	00006h

;
; struct ULS definition
;
grpfULUls	equ	[word ptr 00000h]
fStrikeUls	equ	[byte ptr 00000h]
kulUls		equ	[byte ptr 00001h]
ptUls		equ	[00002h]
xpLimUls	equ	[word ptr 00006h]
hdcUls		equ	[word ptr 00008h]
wwUls		equ	[word ptr 0000Ah]
dypPenUls	equ	[word ptr 0000Ch]
cbUlsMin	equ	0000Eh

;
; struct WWD definition
;
idrMacWwd		equ [word ptr 00000h]
idrMaxWwd		equ [word ptr 00002h]
cbDrWwd			equ [word ptr 00004h]
brgdrWwd		equ [word ptr 00006h]
fExternalWwd		equ [word ptr 00008h]
hpldrBackWwd		equ [word ptr 0000Ah]
wkWwd			equ [word ptr 0000Ch]
fClipBoardWwd		equ [byte ptr 0000Ch]
maskfClipBoardWwd	equ 001h
fMacroWwd		equ [byte ptr 0000Ch]
maskfMacroWwd		equ 002h
fDebugWwd		equ [byte ptr 0000Ch]
maskfDebugWwd		equ 004h
fAtnWwd 		equ [byte ptr 0000Ch]
maskfAtnWwd		equ 008h
fPageViewWwd		equ [byte ptr 0000Dh]
maskfPageViewWwd	equ 008h
fSccWwd 		equ [byte ptr 0000Dh]
maskfSccWwd		equ 010h
fFtnWwd 		equ [byte ptr 0000Dh]
maskfFtnWwd		equ 020h
fHdrWwd 		equ [byte ptr 0000Dh]
maskfHdrWwd		equ 040h
fOutlineWwd		equ [byte ptr 0000Dh]
maskfOutlineWwd 	equ 080h
fDrDirtyWwd		equ [byte ptr 0000Eh]
maskfDrDirtyWwd 	equ 001h
fDirtyWwd		equ [byte ptr 0000Eh]
maskfDirtyWwd		equ 002h
fBackgroundDirtyWwd	equ [byte ptr 0000Eh]
maskfBackgroundDirtyWwd equ 004h
fHadRulerWwd		equ [byte ptr 0000Eh]
maskfHadRulerWwd	equ 008h
fSetElevWwd		equ [byte ptr 0000Eh]
maskfSetElevWwd 	equ 010h
fLowerWwd		equ [byte ptr 0000Eh]
maskfLowerWwd		equ 040h
fEllipWwd		equ [byte ptr 0000Eh]
maskfEllipWwd		equ 080h
fShowFWwd		equ [byte ptr 0000Fh]
maskfShowFWwd		equ 001h
fNeedEnhance		equ [byte ptr 0000Fh]
maskfNeedEnhance	equ 002h
fDisplayAsPrintWwd	equ [byte ptr 0000Fh]
maskFDisplayAsPrintWwd	equ 004h
fNewDrsWwd		equ [byte ptr 0000Fh]
maskfNewDrsWwd		equ 008h

; 1 word of flags
grpfvisiWwd		equ [word ptr 00010h]
;bits in grpfvisi defined under Grpfvisi
hwndWwd 		equ [word ptr 00012h]
hdcWwd			equ [word ptr 00014h]
hwndVScrollWwd		equ [word ptr 00016h]
hkmpCurWwd		equ [word ptr 00018h]
hwndIconBarWwd		equ [word ptr 0001Ah]
hwndPgvUpWwd		equ [word ptr 0001Ch]
hwndPgvDownWwd		equ [word ptr 0001Eh]
mwWwd			equ [word ptr 00020h]
rcwDispWwd		equ [0022h]
xwMinWwd		equ [word ptr 00022h]
ywMinWwd		equ [word ptr 00024h]
xwMacWwd		equ [word ptr 00026h]
ywMacWwd		equ [word ptr 00028h]
rcePageWwd		equ [002Ah]
xeScrollWwd		equ [word ptr 0002Ah]
yeScrollWwd		equ [word ptr 0002Ch]
selsWwd 		equ [0032h]
rcwInvalWwd		equ [0050h]
xwFirstInvalWwd 	equ [word ptr 00050h]
ywFirstInvalWwd 	equ [word ptr 00052h]
xwLimInvalWwd		equ [word ptr 00054h]
ywLimInvalWwd		equ [word ptr 00056h]
wwDispWwd		equ [word ptr 00058h]
dqElevatorWwd		equ [word ptr 0005Ah]
xwSelBarWwd		equ [word ptr 0005Ch]
dxwStyWndWwd		equ [word ptr 0005Eh]
ipgdWwd 		equ [word ptr 00060h]
xhScrollWwd		equ [word ptr 00062h]
xwLimScrollWwd		equ [word ptr 00064h]
wReservedWwd		equ [word ptr 00066h]
rgdrWwd 		equ [0068h]
cbWwdMin	equ	000A6h
cwWWD		equ	(000A6h/00002h)

;
; struct XBC definition
;
ixsMaxXbc	equ	[word ptr 00000h]
ixsMacXbc	equ	[word ptr 00002h]
rgxsXbc 	equ	[word ptr 00004h]
itnMacXbc	equ	[word ptr 00006h]
rgtnXbc 	equ	[00008h]
errnz	<itnMax - 45>
cbXbcMin	equ	00118h
errnz	<(10 + 6 * 45) - cbXbcMin>

;
; struct XSA definition
;
iFirstXsa	equ	[word ptr 00000h]
cXsa		equ	[word ptr 00002h]
cbXsaMin	equ	00004h

;
; struct XSP definition
;
prmXsp			equ	[word ptr 00000h]
fCalledXDeleteHdrText	equ	[word ptr 00002h]
cbXspMin		equ	00004h

;
; struct XSF definition
;
ifooFirstXsf	equ	[word ptr 00000h]
ifooLimXsf	equ	[word ptr 00002h]
cbXsfMin	equ	00004h

;
; struct XSL definition
;
ifldXsl 	equ	[word ptr 00000h]
cfldXsl 	equ	[word ptr 00002h]
cbXslMin	equ	00004h

;
; struct XSB definition
;
ibklLimXsb	equ	[word ptr 00000h]
ibkfLimXsb	equ	[word ptr 00002h]
ibkfXsb 	equ	[word ptr 00004h]
cbXsbMin	equ	00006h

;
; struct XSR definition
;
dfcXsr		    equ     [dword ptr 00000h]
LO_dfcXsr	    equ     [word ptr 00000h]
HI_dfcXsr	    equ     [word ptr 00002h]
fcXsr		    equ     [dword ptr 00004h]
LO_fcXsr	    equ     [word ptr 00004h]
HI_fcXsr	    equ     [word ptr 00006h]
fnXsr		    equ     [word ptr 00008h]
pcaDelXsr	    equ     [word ptr 0000Ah]
pcaInsXsr	    equ     [word ptr 0000Ch]
fNotEmptyPcaDelXsr  equ     [word ptr 0000Eh]
ipcdInsFirstXsr     equ     [word ptr 00010h]
ipcdInsLastXsr	    equ     [word ptr 00012h]
cpcdXsr 	    equ     [word ptr 00014h]
fReplHidnSectXsr    equ     [word ptr 00016h]
fNotDelPassCpMacXsr equ     [word ptr 00018h]
xsfFtnDelXsr	    equ     [0001Ah]
xsfFtnCopyXsr	    equ     [0001Eh]
xsaPgdDelXsr	    equ     [00022h]
xsaPgdCopyXsr	    equ     [00026h]
xsaSedCopyXsr	    equ     [0002Ah]
xsaSeaDelXsr	    equ     [0002Eh]
xsaSeaCopyXsr	    equ     [00032h]
xsaPadDelXsr	    equ     [00036h]
xsaPadCopyXsr	    equ     [0003Ah]
xsaPheDelXsr	    equ     [0003Eh]
xsaPheCopyXsr	    equ     [00042h]
xspXsr		    equ     [00046h]
xsfAtnDelXsr	    equ     [0004Ah]
xsfAtnCopyXsr	    equ     [0004Eh]
xslDeleteXsr	    equ     [00052h]
xsbCopyXsr	    equ     [00056h]
ifdef DEBUG
nxsXsr		    equ     [word ptr 0005Ch]
wXsr		    equ     [word ptr 0005Eh]
cbXsrMin	    equ     00060h
else ;!DEBUG
cbXsrMin	    equ     0005Ch
endif ;!DEBUG
