        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	dialog1_PCODE,dialog1n,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midDialog1n	equ 33		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

ifdef DEBUG
externFP	<S_WCompSzSrt>
externFP	<AssertProcForNative>
endif ;DEBUG


sBegin  data

; EXTERNALS

externW     vitr

sEnd    data

; CODE SEGMENT _EDIT

sBegin	dialog1n
	assumes cs,dialog1n
        assumes ds,dgroup
        assumes ss,dgroup


;#define ch(_i)  ((CHAR) (_i))
;
;csconst CHAR	 mpchordNorm[] = {
mpchordCase:

;/* NUL      SOH      STX      ETX	EOT	 ENQ	  ACK	   BEL	  */
;	 ch(0),   ch(1),   ch(2),   ch(3),   ch(4),   ch(5),   ch(6),	ch(7),
	db	000,001,002,003,004,005,006,007

;/*   BS       HT	LF	 VT	  FF	   CR	    SO	     SI   */
;	 ch(8),   ch(9),   ch(10),  ch(11),  ch(12),  ch(13),  ch(14),	ch(15),
	db	008,009,010,011,012,013,014,015

;/*  DLE      DC1      DC2	DC3	DC4	 NAK	  SYN	   ETB	  */
;	 ch(16),  ch(17),  ch(18),  ch(19),  ch(20),  ch(21),  ch(22),	ch(23),
	db	016,017,018,019,020,021,022,023

;/* CAN      EM       SUB      ESC	FS	 GS	  RS	   US	  */
;	 ch(24),  ch(25),  ch(26),  ch(27),  ch(28),  ch(29),  ch(30),	ch(31),
	db	024,025,026,027,028,029,030,031

;/* SPACE      !	"        #        $        %        &        '    */
;	 ch(32),  ch(33),  ch(34),  ch(35),  ch(36),  ch(37),  ch(38),	ch(39),
	db	032,033,034,035,036,037,038,039

;/*   (        )	*	 +	  ,	   -	    .	     /	  */
;	 ch(40),  ch(41),  ch(42),  ch(43),  ch(44),  ch(45),  ch(46),	ch(47),
	db	040,041,042,043,044,045,046,047

;/*   0        1	2	 3	  4	    5	    6	    7	  */
;	 ch(130), ch(131), ch(132), ch(133), ch(134), ch(135), ch(136), ch(137),
	db	130,131,132,133,134,135,136,137

;/*   8        9	:	 ;	  <	    =	    >	    ?	   */
;	 ch(138), ch(139), ch(48),  ch(49),  ch(50),  ch(51),  ch(52),	ch(53),
	db	138,139,048,049,050,051,052,053

;/*   @        A	B	 C	  D	    E	    F	    G	   */
;	 ch(54),  ch(140), ch(142), ch(144), ch(146), ch(148), ch(150), ch(152),
	db	054,140,142,144,146,148,150,152

;/*   H        I	J	 K	  L	   M	    N	     O	  */
;	 ch(154), ch(156), ch(158), ch(160), ch(162), ch(164), ch(166), ch(170),
	db	154,156,158,160,162,164,166,170

;/*   P        Q	R	 S	  T	   U	    V	     W	  */
;	 ch(172), ch(174), ch(176), ch(178), ch(181), ch(183), ch(185), ch(187),
	db	172,174,176,178,181,183,185,187

;/*   X        Y	Z	 [	  \ 	   ]	    ^	     _	  */
;	 ch(189), ch(191), ch(196), ch(55),  ch(56),  ch(57),  ch(58),	ch(59),
	db	189,191,196,055,056,057,058,059

;/*   `        a	b	 c	  d	   e	    f	     g	  */
;	 ch(60),  ch(141), ch(143), ch(145), ch(147), ch(149), ch(151), ch(153),
	db	060,141,143,145,147,149,151,153

;/*   h        i	j	 k	  l	   m	    n	     o	  */
;	 ch(155), ch(157), ch(159), ch(161), ch(163), ch(165), ch(167), ch(171),
	db	155,157,159,161,163,165,167,171

;/*   p        q	r	 s	  t	   u	    v	     w	  */
;	 ch(173), ch(175), ch(177), ch(179), ch(182), ch(184), ch(186), ch(188),
	db	173,175,177,179,182,184,186,188

;/*   x        y	z	 {	  |	   }	    ~	    DEL   */
;	 ch(190), ch(193), ch(197), ch(61),  ch(62),  ch(63),  ch(64),	ch(65),
	db	190,193,197,061,062,063,064,065

;/*   80       81	82	 83	  84	   85	    86	     87   */
;	 ch(66),  ch(67),  ch(68),  ch(69),  ch(70),  ch(71),  ch(72),	ch(73),
	db	066,067,068,069,070,071,072,073

;/*   88       89	8A	 8B	  8C	   8D	    8E	     8F   */
;	 ch(74),  ch(75),  ch(76),  ch(77),  ch(78),  ch(79),  ch(80),	ch(81),
	db	074,075,076,077,078,079,080,081

;/*   90       91	92	 93	  94	   95	    96	     97   */
;	 ch(82),  ch(83),  ch(84),  ch(85),  ch(86),  ch(87),  ch(88),	ch(89),
	db	082,083,084,085,086,087,088,089

;/*   98       99	9A	 9B	  9C	   9D	    9E	     9F   */
;	 ch(90),  ch(91),  ch(92),  ch(93),  ch(94),  ch(95),  ch(96),	ch(97),
	db	090,091,092,093,094,095,096,097

;/*   A0       A1	A2	 A3	  A4	   A5	    A6	     A7   */
;	 ch(98),  ch(99),  ch(100), ch(101), ch(102), ch(103), ch(104), ch(105),
	db	098,099,100,101,102,103,104,105

;/*   A8       A9	AA	 AB	  AC	   AD	    AE	     AF   */
;	 ch(106), ch(107), ch(108), ch(109), ch(110), ch(111), ch(112), ch(113),
	db	106,107,108,109,110,111,112,113

;/*   B0       B1	B2	 B3	  B4	   B5	    B6	     B7   */
;	 ch(114), ch(115), ch(116), ch(117), ch(118), ch(119), ch(120), ch(121),
	db	114,115,116,117,118,119,120,121

;/*   B8       B9	BA	 BB	  BC	   BD	    BE	     BF   */
;	 ch(122), ch(123), ch(124), ch(125), ch(126), ch(127), ch(128), ch(129),
	db	122,123,124,125,126,127,128,129

;/*   C0       C1	C2	 C3	  C4	   C5	    C6	     C7   */
;	 ch(140), ch(140), ch(140), ch(140), ch(140), ch(140), ch(140), ch(144),
	db	140,140,140,140,140,140,140,144

;/*   C8       C9	CA	 CB	  CC	   CD	    CE	     CF   */
;	 ch(148), ch(148), ch(148), ch(148), ch(156), ch(156), ch(156), ch(156),
	db	148,148,148,148,156,156,156,156

;/*   D0       D1	D2	 D3	  D4	   D5	    D6	     D7   */
;	 ch(146), ch(168), ch(170), ch(170), ch(170), ch(170), ch(170), ch(170),
	db	146,168,170,170,170,170,170,170

;/*   D8       D9	DA	 DB	  DC	   DD	    DE	     DF   */
;	 ch(170), ch(183), ch(183), ch(183), ch(183), ch(192), ch(199), ch(180),
	db	170,183,183,183,183,192,199,180

;/*   E0       E1	E2	 E3	  E4	   E5	    E6	     E7   */
;	 ch(141), ch(141), ch(141), ch(141), ch(141), ch(141), ch(141), ch(145),
	db	141,141,141,141,141,141,141,145

;/*   E8       E9	EA	 EB	  EC	   ED	    EE	     EF   */
;	 ch(149), ch(149), ch(149), ch(149), ch(157), ch(157), ch(157), ch(157),
	db	149,149,149,149,157,157,157,157

;/*   F0       F1	F2	 F3	  F4	   F5	    F6	     F7   */
;	 ch(147), ch(169), ch(171), ch(171), ch(171), ch(171), ch(171), ch(171),
	db	147,169,171,171,171,171,171,171

;/*   F8       F9	FA	 FB	  FC	   FD	    FE	     FF   */
;	 ch(171), ch(184), ch(184), ch(184), ch(184), ch(194), ch(198), ch(195)
	db	171,184,184,184,184,194,198,195

;	 };
;#undef ch

rgchCaseScanExcept:
	db	0C4h,0C5h,0C6h,0D6h,0D8h,0DEh,0E4h,0E5h,0E6h,0F6h,0F8h,0FEh
rgordCaseScanExcept:
	db	0206,0204,0200,0208,0202,0198,0207,0205,0201,0209,0203,0199

	;Case insensitive variants for assembler version only
;#define ch(_i)  ((CHAR) (_i))
;
;csconst CHAR	 mpchordNorm[] = {
mpchordNoCase:

;/* NUL      SOH      STX      ETX	EOT	 ENQ	  ACK	   BEL	  */
;	 ch(0),   ch(1),   ch(2),   ch(3),   ch(4),   ch(5),   ch(6),	ch(7),
	db	000,001,002,003,004,005,006,007

;/*   BS       HT	LF	 VT	  FF	   CR	    SO	     SI   */
;	 ch(8),   ch(9),   ch(10),  ch(11),  ch(12),  ch(13),  ch(14),	ch(15),
	db	008,009,010,011,012,013,014,015

;/*  DLE      DC1      DC2	DC3	DC4	 NAK	  SYN	   ETB	  */
;	 ch(16),  ch(17),  ch(18),  ch(19),  ch(20),  ch(21),  ch(22),	ch(23),
	db	016,017,018,019,020,021,022,023

;/* CAN      EM       SUB      ESC	FS	 GS	  RS	   US	  */
;	 ch(24),  ch(25),  ch(26),  ch(27),  ch(28),  ch(29),  ch(30),	ch(31),
	db	024,025,026,027,028,029,030,031

;/* SPACE      !	"        #        $        %        &        '    */
;	 ch(32),  ch(33),  ch(34),  ch(35),  ch(36),  ch(37),  ch(38),	ch(39),
	db	032,033,034,035,036,037,038,039

;/*   (        )	*	 +	  ,	   -	    .	     /	  */
;	 ch(40),  ch(41),  ch(42),  ch(43),  ch(44),  ch(45),  ch(46),	ch(47),
	db	040,041,042,043,044,045,046,047

;/*   0        1	2	 3	  4	    5	    6	    7	  */
;	 ch(130), ch(131), ch(132), ch(133), ch(134), ch(135), ch(136), ch(137),
	db	130,131,132,133,134,135,136,137

;/*   8        9	:	 ;	  <	    =	    >	    ?	   */
;	 ch(138), ch(139), ch(48),  ch(49),  ch(50),  ch(51),  ch(52),	ch(53),
	db	138,139,048,049,050,051,052,053

;/*   @        A	B	 C	  D	    E	    F	    G	   */
;	 ch(54),  ch(140), ch(142), ch(144), ch(146), ch(148), ch(150), ch(152),
	db	054,141,143,145,147,149,151,153

;/*   H        I	J	 K	  L	   M	    N	     O	  */
;	 ch(154), ch(156), ch(158), ch(160), ch(162), ch(164), ch(166), ch(170),
	db	155,157,159,161,163,165,167,171

;/*   P        Q	R	 S	  T	   U	    V	     W	  */
;	 ch(172), ch(174), ch(176), ch(178), ch(181), ch(183), ch(185), ch(187),
	db	173,175,177,179,182,184,186,188

;/*   X        Y	Z	 [	  \ 	   ]	    ^	     _	  */
;	 ch(189), ch(191), ch(196), ch(55),  ch(56),  ch(57),  ch(58),	ch(59),
	db	190,193,197,055,056,057,058,059

;/*   `        a	b	 c	  d	   e	    f	     g	  */
;	 ch(60),  ch(141), ch(143), ch(145), ch(147), ch(149), ch(151), ch(153),
	db	060,141,143,145,147,149,151,153

;/*   h        i	j	 k	  l	   m	    n	     o	  */
;	 ch(155), ch(157), ch(159), ch(161), ch(163), ch(165), ch(167), ch(171),
	db	155,157,159,161,163,165,167,171

;/*   p        q	r	 s	  t	   u	    v	     w	  */
;	 ch(173), ch(175), ch(177), ch(179), ch(182), ch(184), ch(186), ch(188),
	db	173,175,177,179,182,184,186,188

;/*   x        y	z	 {	  |	   }	    ~	    DEL   */
;	 ch(190), ch(193), ch(197), ch(61),  ch(62),  ch(63),  ch(64),	ch(65),
	db	190,193,197,061,062,063,064,065

;/*   80       81	82	 83	  84	   85	    86	     87   */
;	 ch(66),  ch(67),  ch(68),  ch(69),  ch(70),  ch(71),  ch(72),	ch(73),
	db	066,067,068,069,070,071,072,073

;/*   88       89	8A	 8B	  8C	   8D	    8E	     8F   */
;	 ch(74),  ch(75),  ch(76),  ch(77),  ch(78),  ch(79),  ch(80),	ch(81),
	db	074,075,076,077,078,079,080,081

;/*   90       91	92	 93	  94	   95	    96	     97   */
;	 ch(82),  ch(83),  ch(84),  ch(85),  ch(86),  ch(87),  ch(88),	ch(89),
	db	082,083,084,085,086,087,088,089

;/*   98       99	9A	 9B	  9C	   9D	    9E	     9F   */
;	 ch(90),  ch(91),  ch(92),  ch(93),  ch(94),  ch(95),  ch(96),	ch(97),
	db	090,091,092,093,094,095,096,097

;/*   A0       A1	A2	 A3	  A4	   A5	    A6	     A7   */
;	 ch(98),  ch(99),  ch(100), ch(101), ch(102), ch(103), ch(104), ch(105),
	db	098,099,100,101,102,103,104,105

;/*   A8       A9	AA	 AB	  AC	   AD	    AE	     AF   */
;	 ch(106), ch(107), ch(108), ch(109), ch(110), ch(111), ch(112), ch(113),
	db	106,107,108,109,110,111,112,113

;/*   B0       B1	B2	 B3	  B4	   B5	    B6	     B7   */
;	 ch(114), ch(115), ch(116), ch(117), ch(118), ch(119), ch(120), ch(121),
	db	114,115,116,117,118,119,120,121

;/*   B8       B9	BA	 BB	  BC	   BD	    BE	     BF   */
;	 ch(122), ch(123), ch(124), ch(125), ch(126), ch(127), ch(128), ch(129),
	db	122,123,124,125,126,127,128,129

;/*   C0       C1	C2	 C3	  C4	   C5	    C6	     C7   */
;	 ch(140), ch(140), ch(140), ch(140), ch(140), ch(140), ch(140), ch(144),
	db	141,141,141,141,141,141,141,145

;/*   C8       C9	CA	 CB	  CC	   CD	    CE	     CF   */
;	 ch(148), ch(148), ch(148), ch(148), ch(156), ch(156), ch(156), ch(156),
	db	149,149,149,149,157,157,157,157

;/*   D0       D1	D2	 D3	  D4	   D5	    D6	     D7   */
;	 ch(146), ch(168), ch(170), ch(170), ch(170), ch(170), ch(170), ch(170),
	db	147,169,171,171,171,171,171,171

;/*   D8       D9	DA	 DB	  DC	   DD	    DE	     DF   */
;	 ch(170), ch(183), ch(183), ch(183), ch(183), ch(192), ch(199), ch(180),
	db	171,184,184,184,184,194,198,180

;/*   E0       E1	E2	 E3	  E4	   E5	    E6	     E7   */
;	 ch(141), ch(141), ch(141), ch(141), ch(141), ch(141), ch(141), ch(145),
	db	141,141,141,141,141,141,141,145

;/*   E8       E9	EA	 EB	  EC	   ED	    EE	     EF   */
;	 ch(149), ch(149), ch(149), ch(149), ch(157), ch(157), ch(157), ch(157),
	db	149,149,149,149,157,157,157,157

;/*   F0       F1	F2	 F3	  F4	   F5	    F6	     F7   */
;	 ch(147), ch(169), ch(171), ch(171), ch(171), ch(171), ch(171), ch(171),
	db	147,169,171,171,171,171,171,171

;/*   F8       F9	FA	 FB	  FC	   FD	    FE	     FF   */
;	 ch(171), ch(184), ch(184), ch(184), ch(184), ch(194), ch(198), ch(195)
	db	171,184,184,184,184,194,198,195

;	 };
;#undef ch

rgchNoCaseScanExcept:
	db	0C4h,0C5h,0C6h,0D6h,0D8h,0DEh,0E4h,0E5h,0E6h,0F6h,0F8h,0FEh
rgordNoCaseScanExcept:
	db	0207,0205,0201,0209,0203,0199,0207,0205,0201,0209,0203,0199

ifdef DEBUG
cProc	N_OrdCh,<PUBLIC,FAR,ATOMIC>,<di>
	ParmW	chArg
	ParmW	fCase
	ParmW	fScan

cBegin
	mov	al,bptr [chArg]
	mov	bx,offset [mpchordCase]
	cmp	[fCase],fFalse
	jne	OC01
	mov	bx,offset [mpchordNoCase]
OC01:
	cmp	[fScan],fFalse
	je	OC02
	push	cs
	pop	es
	errnz	<offset [rgchCaseScanExcept] - offset [mpchordCase] - 256>
	errnz	<offset [rgchNoCaseScanExcept] - offset [mpchordNoCase] - 256>
	lea	di,[bx + 256]
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	cx,12
	repne	scasb
	jne	OC02
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	al,cs:[di+12-1]
	db	03Dh	;turns next "xlat cs:[bx]" into "cmp ax,immediate"
OC02:
	xlat	cs:[bx]
OC03:
	xor	ah,ah
cEnd
endif ;DEBUG

;-------------------------------------------------------------------------
;	WCompSzSrt(psz1,psz2,fCase)
;-------------------------------------------------------------------------
;-- Routine: WCompSzSrt(psz1,psz2,fCase)
;-- Description and Usage:
;	 Alphabetically compares the two null-terminated strings lpsz1 and lpsz2.
;	 Upper case alpha characters are mapped to lower case iff fCase
;	 Comparison of non-alpha characters is by ascii code.
;	 Returns 0 if they are equal, a negative number if lpsz1 precedes lpsz2, and
;		 a non-zero positive number if lpsz2 precedes lpsz1.
;	 If 2 strings compare equal, we do an ansi compare and return that value
;		 so we handle Apple befor apple and strings identical except
;		 for accents (intl).
;    We now set negative values so that prefixes and < are negative but
;	 distinct, so callers who care about prefix get the info, but
;	 others can just check < 0.
;-- Arguments:
;	 psz1, psz2  - pointers to two null-terminated strings to compare
;-- Returns:
;	 a short     - 0 if strings are equal, -2 if psz1 precedes psz2,
;	 -1 if psz1 is a prefix of psz2,
;		 and non-zero positive number if psz2 precedes psz1.
;-- Side-effects: none
;-- Bugs:
;-- History:
;	 3/14/83     - created (tsr)
;----------------------------------------------------------------------------*/
;WCompSzSrt(psz1,psz2,fCase)
;register PCH psz1;
;register PCH psz2;
;int fCase;
;{
;	int ord1;
;	int ord2;
;	int dch;
;	PCH pszOrig1 = psz1;
;	PCH pszOrig2 = psz2;
;	CHAR FAR *mpchord = vitr.fScandanavian ? mpchordScan : mpchordNorm;

WCSS01:
	push	di
	errnz	<offset [rgchCaseScanExcept] - offset [mpchordCase] - 256>
	errnz	<offset [rgchNoCaseScanExcept] - offset [mpchordNoCase] - 256>
	lea	di,[bx + 256]
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	cx,12
	repne	scasb
	jne	WCSS02
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	al,cs:[di+12-1]
	db	03Dh	;turns next "xlat cs:[bx]" into "cmp ax,immediate"
WCSS02:
	xlat	cs:[bx]
	pop	di
	jmp	short WCSS07

WCSS03:
	push	di
	errnz	<offset [rgchCaseScanExcept] - offset [mpchordCase] - 256>
	errnz	<offset [rgchNoCaseScanExcept] - offset [mpchordNoCase] - 256>
	lea	di,[bx + 256]
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	cx,12
	repne	scasb
	jne	WCSS04
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	al,cs:[di+12-1]
	db	03Dh	;turns next "xlat cs:[bx]" into "cmp ax,immediate"
WCSS04:
	xlat	cs:[bx]
	pop	di
	jmp	short WCSS08


cProc	N_WCompSzSrt,<PUBLIC,FAR,ATOMIC>,<si,di>
	ParmW	psz1
	ParmW	psz2
	ParmW	fCase

cBegin
	xor	ah,ah
	push	cs
	pop	es
	mov	si,[psz1]
	mov	di,[psz2]
	mov	bx,offset [mpchordCase]
	cmp	[fCase],fFalse
	jne	WCSS05
	mov	bx,offset [mpchordNoCase]
WCSS05:
	mov	dl,[vitr.fScandanavianItr]
	or	dl,dl
	je	WCSS06
	mov	dl,080h
WCSS06:

;	/* Note: ChLower used intentionally, rather than ChUpper so that
;		French comparisons use the sort table values, not the stripped
;		upper case values bz
;	*/
;
;	for (ord1 = mpchord[fCase ? *psz1++ : ChLower(*psz1++)],
;	     ord2 = mpchord[fCase ? *psz2++ : ChLower(*psz2++)];
;	     ord1 == ord2;
;	     ord1 = mpchord[fCase ? *psz1++ : ChLower(*psz1++)],
;	     ord2 = mpchord[fCase ? *psz2++ : ChLower(*psz2++)])
;		{
	mov	al,[di]
	inc	di
	test	al,dl
	jne	WCSS01
	xlat	cs:[bx]
WCSS07:
	mov	dh,al
	lodsb
	test	al,dl
	jne	WCSS03
	xlat	cs:[bx]
WCSS08:
	cmp	al,dh
	jne	WCSS09

;		if (ord1 == '\0')
;			{
	cmp	al,0
	jne	WCSS06

;			psz1 = pszOrig1;
;			psz2 = pszOrig2;
;			for (ord1=*psz1++, ord2=*psz2++; ord1==ord2; ord1=*psz1++, ord2=*psz2++)
;				{
;				if (ord1 == '\0')
;					return(0);
;				}
;			break;
;			}
	xor	al,al
	mov	cx,si
	mov	si,[psz1]
	sub	cx,si
	mov	di,[psz2]
	push	ds
	pop	es
	repe	cmpsb
	je	WCSS10
	mov	al,[si-1]
	mov	dh,[di-1]

;		}
WCSS09:

;	if ((dch = ord1 - ord2) > 0)
;		return dch;
	mov	dl,al
	sub	al,dh
	ja	WCSS10

;	else  if (ord1 != '\0')
;		return -2;  /* psz1 < psz2 */
;	else
;		return -1;  /* != but *psz1 == 0; psz1 is a prefix of psz2 */
        cmp	dl,1
	cmc
	sbb	ax,ax
	dec	ax

;} /* end of  W C o m p S z Srt  */
WCSS10:
cEnd


;-------------------------------------------------------------------------
;	WCompChCh (ch1, ch2)
;-------------------------------------------------------------------------
;/* W  C O M P	C H  C H */
;/*  Compare two characters using the table in this module.  Return zero
;	 if ch1 == ch2, negative if ch1 < ch2, and positive if ch1 > ch2.
;*/
;WCompChCh (ch1, ch2)
;CHAR ch1, ch2;
;
;{
;	 CHAR FAR *mpchord = vitr.fScandanavian ? mpchordScan : mpchordNorm;
;	 return mpchord[ch1] - mpchord[ch2];
;}
cProc	N_WCompChCh,<PUBLIC,FAR,ATOMIC>,<di>
	ParmW	ch1
	ParmW	ch2

cBegin
	push	cs
	pop	es
	mov	bx,offset [mpchordCase]
	xor	ah,ah
	mov	al,bptr [ch2]
	call	WCCC01
	mov	dx,ax
	mov	al,bptr [ch1]
	call	WCCC01
	sub	ax,dx
cEnd

WCCC01:
	cmp	[vitr.fScandanavianItr],fFalse
	je	WCCC02
	errnz	<offset [rgchCaseScanExcept] - offset [mpchordCase] - 256>
	errnz	<offset [rgchNoCaseScanExcept] - offset [mpchordNoCase] - 256>
	lea	di,[bx + 256]
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	cx,12
	repne	scasb
	jne	WCCC02
	errnz	<offset [rgordCaseScanExcept] - offset [rgchCaseScanExcept] - 12>
	errnz	<offset [rgordNoCaseScanExcept] - offset [rgchNoCaseScanExcept] - 12>
	mov	al,cs:[di+12-1]
	db	03Dh	;turns next "xlat cs:[bx]" into "cmp ax,immediate"
WCCC02:
	xlat	cs:[bx]
	ret


;/* L b c  C m p  L b o x */
;/* from lbox.h */
;#define lbcEq		 0
;#define lbcPrefix	 1
;#define lbcLt		 2
;#define lbcGt		 3
;
;/*  Callback compare rtn for sdm listboxes
;    Compare two characters using the table in this module.
;    Return lbc codes above
;*/
;EXPORT LbcCmpLbox (wUser, pstz1, pstz2)
;WORD wUser;
;CHAR **pstz1;
;CHAR **pstz2;
;{
;	 int w;

lbcEq	    equ    0
lbcPrefix   equ    1
lbcLt	    equ    2
lbcGt	    equ    3

cProc	LbcCmpLbox,<PUBLIC,FAR,ATOMIC>,<>
	ParmW	wUser
	ParmW	pstz1
	ParmW	pstz2

cBegin

;	 w = WCompSzSrt(*pstz1 + 1,*pstz2 + 1, fFalse);  /* skip over st size */
	mov	bx,[pstz1]
	mov	bx,[bx]
	inc	bx
	push	bx
	mov	bx,[pstz2]
	mov	bx,[bx]
	inc	bx
	push	bx
	errnz	<fFalse>
	xor	bx,bx
	push	bx
ifdef DEBUG
	cCall	S_WCompSzSrt,<>
else ;not DEBUG
	push	cs
	call	near ptr N_WCompSzSrt
endif ;DEBUG

;	 if (w == 0)
;		 {
;		 return (lbcEq);
;		 }
;	 else  if (w > 0)
;		 return (lbcGt);
;	 else  if (w == -2)
;		 return (lbcLt);
;	 else
;		 {
;		 Assert (w == -1);
;		 return (lbcPrefix);
;		 }
;}
        errnz   <lbcEq>
        errnz   <lbcPrefix - 1>
        errnz   <lbcLt - 2>
	neg	ax
ifdef DEBUG
	cmp	ax,2
	jle	LCL01
	push	ax
	push	bx
	push	cx
	push	dx
	mov	ax,midDialog1n
	mov	bx,1001
	cCall	AssertProcForNative,<ax,bx>
	pop	dx
	pop	cx
	pop	bx
	pop	ax
LCL01:
	or	ax,ax
endif ;DEBUG
	jge	LCL02
	mov	ax,lbcGt
LCL02:

cEnd


sEnd	dialog1n
        end
