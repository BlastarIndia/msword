; consts.inc - constants for hand native 
;
; WARNING: these constants MUST be updated in sync with their
;          counterparts in the .n files 
; Please try to keep this file in alphabetical order 

bptr		equ	byte ptr
wptr		equ	word ptr
wlo		equ	word ptr
whi		equ	2 + word ptr

bkcNewPage	equ	2
bNil		equ	0
bpmCp		equ	0
bpmFoo		equ	1
brclSingle	equ	0
brclThick	equ	1
brclDouble	equ	2
brclShadow	equ	3
brcpNone	equ	0

brcNone 	equ	0
brcBox		equ	15
brcBar		equ	16
brcAbove	equ	1
brcBelow	equ	2
brcNil		equ	-1
brcSingle	equ	00040h
brcTwoSingle	equ	00049h
brcThick	equ	00080h
brcDotted       equ     00180h
brcHairline     equ     001C0h

; from file.h
cbSector	equ	512 	        ; assumed to be = 512 in CachePara
maskSector	equ	001FFh
shftSector	equ	9		; assumed to be = 9 in CachePara

cchMaxFile      equ     120

cchInsertMax	equ	32
cchMaxEntry	equ	256
cchMaxRibl	equ	256
cchPapxMax	equ	502
cchSearchMax	equ	257
ccpEop          equ     2
ccpSect 	equ	1

chColumnBreak	equ	14
chFieldBegin    equ     19
chFieldSeparate equ     20
chFieldEnd      equ     21
chFieldMin      equ     19
chFieldMax      equ     22
chFieldBeginDisp	equ	123
chFieldSeparateDisp     equ     124
chFieldEndDisp	        equ	125
chFormula	equ	92
chDisplayField  equ     17
chTable 	equ	7
chPicture	equ	1
chVisCellEnd	equ	0A4H

chDxpMin    equ 0
chDxpMax    equ 256
chCRJ		equ	11
chEol		equ	10
chEop		equ	10
chFootnote	equ	2
chHyphen	equ	45
chNil		equ	(-1)
chNonBreakHyphen	equ	30
chNonBreakSpace	equ	160
chNonReqHyphen	equ	31
chReturn	equ	13
chSect		equ	12
chSpace		equ	32
chTab		equ	9
chTFtn         equ  3
chTFtnCont     equ  4
chVisCRJ	        equ	000ACh
chVisEop	        equ	000B6h
chVisSpace              equ     000B7h
chVisNonReqHyphen       equ     00ACh
chVisNonBreakSpace      equ     00B0h

chPubMin	equ	145
chPubLQuote	equ	145
chPubRQuote	equ	146
chPubLDblQuote	equ	147
chPubRDblQuote	equ	148
chPubBullet	equ	149
chPubEmDash	equ	150
chPubEnDash	equ	151
chPubMax	equ	152

chsMac		equ	0
chsPC		equ	1
chsAnsi 	equ	2
chsPCA		equ	3

clMax		equ	07FFFh
clMaxLbs	equ	400
clNil		equ	-1

LO_cpMax	equ	0FFFFh
HI_cpMax	equ	07FFFh
LO_cpNil	equ	0FFFFh
HI_cpNil	equ	0FFFFh
LO_cpWarnTooBig equ	00000h
HI_cpWarnTooBig equ	00100h

czaInch		equ	1440
czaPoint	equ	20
cptInch		equ	72

DEFAULT_PITCH	equ	0
DEVICEDEFAULT_FONT  equ 14
dkGlsy		equ	4
dlkEnd		equ	1
dlkEndPage	equ	2
dlkEndDr	equ	4
dlkNil		equ	0
docNew		equ	1
docNil		equ	0
docUndo 	equ	3
docScrap	equ	2
dxaInch		equ	1440
dyaPoint	equ	20

dxpChVisEach	equ	12
dxpChVis	equ	24
dypChVis	equ	7

edcNone 	equ	0
eidDocTooBigToSave	equ	129
eidCpRollOver		equ	130
eidNoMemOperation	equ	 16
eidSysLock		equ	160
eidSysFull		equ	161

emkBlank    equ     0
emkEndmark  equ     1
emkSplat    equ     2

fcidNil_LO  equ 0ffffh
fcidNil_HI  equ 0ffffh

fcSpecEop	equ	100
fcSpecNoRoom	equ	102
LO_fcMax	equ	00000h
HI_fcMax	equ	00100h

fcmChars	equ	08000h
fcmParseCaps	equ	1
fcmProps	equ	00080h

fFalse		equ	0
fTrue		equ	1

ffcSkip         equ     0
ffcShow         equ     1
ffcDisplay      equ     2
ffcBeginGroup   equ     3
ffcEndGroup     equ     4
ffcWrap         equ     5
ffcRestart      equ     6

ffeNested	equ	1
ffeNoFSpec	equ	2

FIXED_PITCH     equ     1

flmDisplay	    equ     0
flmDisplayAsPrint   equ     1
flmPrint	    equ     2
flmRepaginate	    equ     3
flmIdle 	    equ     4
flmTossPrinter	    equ     5

fltMin		equ	0
fltFormula      equ     49
fltMax		equ	56
fltSeqLevNum	equ	54
fltSeqLevOut	equ	52
fmtList         equ     5

fnInsert	equ	0
fnNil		equ	0
fnScratch       equ     1
fnSpec		equ	0
fnMax		equ	50

fpcEndnote	equ	0
fpcBottomPage	equ	1
fpcBeneathText	equ	2
fpcEndDoc	equ	3

fvcmFields  	equ	00100h
fvcmProps   	equ	00400h
fvcmWw	    	equ	000ffh
fvcInstructions equ	08100h
fvcResults  	equ	08000h
fvcHiddenVisi	equ	08400h

FW_DONTCARE	equ     0
FW_THIN 	equ     100
FW_EXTRALIGHT	equ     200
FW_LIGHT	equ     300
FW_NORMAL	equ     400
FW_MEDIUM	equ     500
FW_SEMIBOLD	equ     600
FW_BOLD 	equ     700
FW_EXTRABOLD	equ     800
FW_HEAVY	equ     900
FW_ULTRALIGHT	equ     FW_EXTRALIGHT
FW_REGULAR	equ     FW_NORMAL
FW_DEMIBOLD	equ     FW_SEMIBOLD
FW_ULTRABOLD	equ     FW_EXTRABOLD
FW_BLACK	equ     FW_HEAVY

GMEM_NOT_BANKED equ	01000h
GW_HWNDFIRST	equ	0
GW_HWNDLAST	equ	1
GW_HWNDNEXT	equ	2
GW_HWNDPREV	equ	3
GW_OWNER	equ	4
GW_CHILD	equ	5

hfontSpecNil	equ	1
hNil        	equ 	0
hpsDefault	equ	20
hpsPosNil   	equ 	128

ibpNil      equ -1

ibrcTop     equ 0
ibrcLeft    equ 1
ibrcBottom  equ 2
ibrcRight   equ 3
ibrcInside  equ 4

ibstFontMax equ 255
ibstFontNil equ ibstFontMax        
ibstCourier equ 3
ibstNil     equ -1

icoAuto 	equ	0
icoRTFMax	equ	50

ichCapsMax	equ	20

ichMaxLine	equ	255
ichNil		equ	255
ichUsrInitlMax	equ	6

idcbChVisCRJ	equ	0
idcbChVisTab	equ	1
idcbMaxChVis	equ	2
idcbOtlMarkPlus equ	0
idcbOtlMarkMinus equ	2
idcbOtlMarkBody	equ	4
idrNil			equ	-1
idrbChVis		equ	0
ifceMax 	equ	32
ifcFkpNil	equ	128	;(512 /*cbSector*/ / sizeof(FC))
ifldNil 	equ	-1
ifmaMax 	equ	40
ihdtMaxSep	equ	6
ihdtNil 	equ	-1
ihdtTFtn	equ	6
ilrNil		equ	-1
iNil		equ	-1
ipatVertGray	equ	0
ipatVertBlack	equ	1
ipatHorzGray	equ	0
ipatHorzLtGray	equ	1
ipatHorzBlack	equ	2
iPicMacPict	equ	2
irmBarNone	equ	0
irmBarLeft	equ	1
irmBarRight	equ	2
irmBarOutside	equ	3
irmPropsNone		equ	0
irmPropsBold		equ	1
irmPropsItalic		equ	2
irmPropsUnder		equ	3
irmPropsDblUnder	equ	4

irtnAssignLr    equ     0
irtnFormPara    equ     1
irtnFormChain   equ     2

iselsInsertMax	equ	4
ispAdjustCp	equ	5
ispInsertLine	equ	3
ispFieldFormat1 equ	4
ispDisplayFli	equ	5
ispFormatLine	equ	5
ispLayout3	equ	9
ispLayout4	equ	8
ispWireFont	equ	7
itbdMax 	equ	50
itcMax		equ	32
itcxcMax	equ	10
itnMax		equ	45

jcLeft		equ	0
jcCenter	equ	1
jcRight		equ	2
jcBoth		equ	3
jcDecimal	equ	3
jcBar		equ	4
jcDecTable	equ	5

kulNone		equ	0
kulSingle	equ	1
kulWord		equ	2
kulDouble	equ	3
kulDotted	equ	4

lbcNil			equ	0
lbcPageBreakBefore	equ	1
lbcEndOfPage		equ	2
lbcEndOfSection 	equ	3
lbcEndOfDoc		equ	4
lbcYlLim		equ	5
lbcEndOfColumn		equ	6
lbcAbort		equ	7
LF_FACESIZE	equ	32
lmNil		equ	0
lmPrint 	equ	1
lmPreview	equ	2
lmFRepag	equ	3
lmBRepag	equ	4
lmPagevw	equ	5
lnnNil		equ	-1
lrkNormal	equ	0
lrkAbs		equ	1
lrkAbsHit	equ	2
lrkSxs		equ	3
lrsNormal	equ	0
lrsFrozen	equ	1
lrsIgnore	equ	2
lvlBody 	equ	9
lvlUpdate	equ	15

maskFNonUser	equ	00314h
maskPrqLF	equ	003h
maskFVarPitchTM	equ	001h
maskFfFfid      equ     070h
matNil		equ	0
matLow		equ	1
matReplace	equ	2
matMem		equ	3
matDisp 	equ	4
matFont         equ     6
mtyAny		equ	000h
mtyNoTimer	equ	001h
mtyYield	equ	004h
mtyDoBlink	equ	008h
mtyAllPeek	equ	010h
mtySomePeek	equ	020h
mtyMouseMove	equ	040h
mtyIdle 	equ	mtyDoBlink OR mtyYield OR mtySomePeek OR mtyMouseMove
mtyUpdateWw	equ	mtyNoTimer
mtyTyping	equ	mtyNoTimer OR mtySomePeek
mwMax		equ	15
mwNil		equ	0

NULL		equ	0
nxsHidnSect	equ	1

omkNil	    equ     0
omkPlus	    equ	    1
omkMinus    equ	    2
omkBody     equ     3

paInvert	equ	0
paDotted	equ	1
pcVMargin	equ	0
pcVPage 	equ	1
pcHColumn	equ	0
pcHMargin	equ	1
pcHPage 	equ	2
pisNormal	equ	0
pltHorz 	equ	000h
pltVert 	equ	001h
pltOnePixel	equ	002h
pltInvert	equ	004h
pltDouble	equ	008h
pnNil		equ	-1
prmNil		equ	0

LO_PATCOPY	equ	00021h
HI_PATCOPY	equ	000f0h
rdsMain 	equ	0
rdsPic		equ	1
rdsStylesheet	equ	4
rdsFonttbl	equ	5
rdsColortbl	equ	6
rdsPrivate1	equ	8
rdsFldType	equ	12
rdsBkmkEnd	equ	29
rdsBkmkStart	equ	30
rdsGridtbl	equ	35
rdsXe		equ	36
rdsTc		equ	37
risNorm 	equ	0
risExit 	equ	1
risB4ContSeq	equ	2
risContWord	equ	3
risNumScan	equ	4
risScanPic	equ	5
risB4Private1	equ	6
risB4BinCode	equ	7
risBinCode	equ	8
risB4SpecXeTc	equ	9
risB4TableName	equ	10
risScanTableName equ	11
risAfContSeq	equ	12
risScanByDest	equ	13
risDoPop	equ	14
risAddTableName equ	15
rzOtlPat	equ	9

sasMin		equ 8
sasOK		equ	3
sasFull 	equ	0

sbDds		equ	1

sgcPap		equ	0
sgcChp		equ	1
sgcSep		equ	2
sgcPic		equ	3
sgcTap		equ	4


skNil		equ	010h
skBlock 	equ	008h
skGraphics	equ	004h
skSelConst	equ	000h
skIns		equ	020h
skRows		equ	002h
skColumn	equ	00Bh

; these spra values are assumed to have these values in ApplyPrlSgc() in fetchn.asm
spraBit         equ     0
spraByte        equ     1
spraWord        equ     2
spraCPlain      equ     3
spraCFtc        equ     4
spraCKul        equ     5
spraCSizePos    equ     6
spraSpec        equ     7
spraCIco        equ     8
spraCHpsInc	equ	9
spraCHpsPosAdj	equ	10

sprmNoop	equ	0
sprmPStcPermute equ	3
sprmPChgTabs	equ	23
sprmTDefTable	equ	152
sprmCQpsSpace	equ	71
sprmPStc	equ	2
sprmTDxaGapHalf equ	148
sprmPPc 	equ	29
sprmCDefault	equ	57
sprmMax         equ     164  ; keep in sync with value in prm.h!! 

stcNormal   equ 0
stcStdMin   equ 222
stcNil	    equ 256
SYSTEM_FONT equ 13

tlcNone		equ	0
tlcDot		equ	1
tlcHyphen	equ	2
tlcSingle	equ	3
tlcHeavy	equ	4
tlcDefault	equ	tlcNone
tsMax		equ	65535
tPos		equ	1
tZero		equ	0
tNeg		equ	(-1)
tntNil		equ	0
tntHplc 	equ	1
tntHsttb	equ	2
tntAbort	equ	3
tntCloseHdrWw	equ	4
tntDelBkmk	equ	5

udmodNormal	equ	0
udmodLastDr	equ	1
udmodTable	equ	2
udmodNoDisplay	equ	3
udmodLimit	equ	4

VARIABLE_PITCH      equ     2
vcparaAbortMax	    equ     4

wkHdr		equ	04000h
wMatchAny	equ	0073Fh
wMatchWhite	equ	00777h
wwNil		equ	0
wwTemp		equ	1
wwPreview	equ	2
wwLayout	equ	3
wwDocMin	equ	5

xaRightMaxSciConst    equ     (22*dxaInch)
xpNil		equ	08000h
xpSplat		equ	16384

wwMax		equ	30

ylLarge 	equ	03FFFh
LO_yllLarge	equ	0FFFFh
HI_yllLarge	equ	07FFFh
ypLarge 	equ	03FFFh

; ilcds for CmdLookn.asm
ilcdBold        equ      (0)
ilcdItalic      equ      (1)
ilcdSmallCaps   equ      (2)
ilcdVanish      equ      (3)  
ilcdKulSingle   equ      (4)
ilcdKulWord     equ      (5)
ilcdKulDouble   equ      (6)
ilcdSuperscript equ      (7)
ilcdSubscript   equ      (8)
ilcdShowAll     equ      (9)
ilcdPlainText   equ      (10)
ilcdStrike      equ      (11)
ilcdRMark       equ      (12)
ilcdParaLeft    equ      (13)
ilcdParaCenter  equ      (14)
ilcdParaRight   equ      (15)
ilcdParaBoth    equ      (16)
ilcdSpace1      equ      (17)
ilcdSpace15     equ      (18)
ilcdSpace2      equ      (19)
ilcdParaClose   equ      (20)
ilcdParaOpen    equ      (21)
ilcdParaNormal  equ      (22)

cmdOK	    	equ	0

mstDbgInt3  	equ	1007
