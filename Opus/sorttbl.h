#ifdef DEBUG
/* Contains a table to map an ANSI character to an ordinal number. 
	Used in WCompSzSrt() and WCompCaseSzSrt().  It is very important that 
	the table maps the null character (\0) to 0 itself.   

	Note that a number of characters, such as accented a's, map to the same 
	value.
*/

#define ch(_i)  ((CHAR) (_i))

csconst CHAR    mpchordNorm[] = {
/* NUL      SOH      STX      ETX      EOT      ENQ      ACK      BEL    */
	ch(0),   ch(1),   ch(2),   ch(3),   ch(4),   ch(5),   ch(6),   ch(7),
/*   BS       HT       LF       VT       FF       CR       SO       SI   */
	ch(8),   ch(9),   ch(10),  ch(11),  ch(12),  ch(13),  ch(14),  ch(15),

/*  DLE      DC1      DC2      DC3     DC4      NAK      SYN      ETB    */
	ch(16),  ch(17),  ch(18),  ch(19),  ch(20),  ch(21),  ch(22),  ch(23),
/* CAN      EM       SUB      ESC      FS       GS       RS       US     */
	ch(24),  ch(25),  ch(26),  ch(27),  ch(28),  ch(29),  ch(30),  ch(31),

/* SPACE      !        "        #        $        %        &        '    */
	ch(32),  ch(33),  ch(34),  ch(35),  ch(36),  ch(37),  ch(38),  ch(39),
/*   (        )        *        +        ,        -        .        /    */
	ch(40),  ch(41),  ch(42),  ch(43),  ch(44),  ch(45),  ch(46),  ch(47),

/*   0        1        2        3        4         5       6       7     */
	ch(130), ch(131), ch(132), ch(133), ch(134), ch(135), ch(136), ch(137),
/*   8        9        :        ;        <         =       >       ?      */
	ch(138), ch(139), ch(48),  ch(49),  ch(50),  ch(51),  ch(52),  ch(53),

/*   @        A        B        C        D         E       F       G      */
	ch(54),  ch(140), ch(142), ch(144), ch(146), ch(148), ch(150), ch(152),
/*   H        I        J        K        L        M        N        O    */
	ch(154), ch(156), ch(158), ch(160), ch(162), ch(164), ch(166), ch(170),

/*   P        Q        R        S        T        U        V        W    */
	ch(172), ch(174), ch(176), ch(178), ch(181), ch(183), ch(185), ch(187),
/*   X        Y        Z        [        \        ]        ^        _    */
	ch(189), ch(191), ch(196), ch(55),  ch(56),  ch(57),  ch(58),  ch(59),

/*   `        a        b        c        d        e        f        g    */
	ch(60),  ch(141), ch(143), ch(145), ch(147), ch(149), ch(151), ch(153),
/*   h        i        j        k        l        m        n        o    */
	ch(155), ch(157), ch(159), ch(161), ch(163), ch(165), ch(167), ch(171),

/*   p        q        r        s        t        u        v        w    */
	ch(173), ch(175), ch(177), ch(179), ch(182), ch(184), ch(186), ch(188),
/*   x        y        z        {        |        }        ~       DEL   */
	ch(190), ch(193), ch(197), ch(61),  ch(62),  ch(63),  ch(64),  ch(65),

/*   80       81       82       83       84       85       86       87   */
	ch(66),  ch(67),  ch(68),  ch(69),  ch(70),  ch(71),  ch(72),  ch(73),
/*   88       89       8A       8B       8C       8D       8E       8F   */
	ch(74),  ch(75),  ch(76),  ch(77),  ch(78),  ch(79),  ch(80),  ch(81),

/*   90       91       92       93       94       95       96       97   */
	ch(82),  ch(83),  ch(84),  ch(85),  ch(86),  ch(87),  ch(88),  ch(89),
/*   98       99       9A       9B       9C       9D       9E       9F   */
	ch(90),  ch(91),  ch(92),  ch(93),  ch(94),  ch(95),  ch(96),  ch(97),

/*   A0       A1       A2       A3       A4       A5       A6       A7   */
	ch(98),  ch(99),  ch(100), ch(101), ch(102), ch(103), ch(104), ch(105),
/*   A8       A9       AA       AB       AC       AD       AE       AF   */
	ch(106), ch(107), ch(108), ch(109), ch(110), ch(111), ch(112), ch(113),

/*   B0       B1       B2       B3       B4       B5       B6       B7   */
	ch(114), ch(115), ch(116), ch(117), ch(118), ch(119), ch(120), ch(121),
/*   B8       B9       BA       BB       BC       BD       BE       BF   */
	ch(122), ch(123), ch(124), ch(125), ch(126), ch(127), ch(128), ch(129),

/*   C0       C1       C2       C3       C4       C5       C6       C7   */
	ch(140), ch(140), ch(140), ch(140), ch(140), ch(140), ch(140), ch(144),
/*   C8       C9       CA       CB       CC       CD       CE       CF   */
	ch(148), ch(148), ch(148), ch(148), ch(156), ch(156), ch(156), ch(156),

/*   D0       D1       D2       D3       D4       D5       D6       D7   */
	ch(146), ch(168), ch(170), ch(170), ch(170), ch(170), ch(170), ch(170),
/*   D8       D9       DA       DB       DC       DD       DE       DF   */
	ch(170), ch(183), ch(183), ch(183), ch(183), ch(192), ch(199), ch(180),

/*   E0       E1       E2       E3       E4       E5       E6       E7   */
	ch(141), ch(141), ch(141), ch(141), ch(141), ch(141), ch(141), ch(145),
/*   E8       E9       EA       EB       EC       ED       EE       EF   */
	ch(149), ch(149), ch(149), ch(149), ch(157), ch(157), ch(157), ch(157),

/*   F0       F1       F2       F3       F4       F5       F6       F7   */
	ch(147), ch(169), ch(171), ch(171), ch(171), ch(171), ch(171), ch(171),
/*   F8       F9       FA       FB       FC       FD       FE       FF   */
	ch(171), ch(184), ch(184), ch(184), ch(184), ch(194), ch(198), ch(195)
	};

	/* Scandanavian table only! Slightly different order */

csconst CHAR    mpchordScan[] = {
/* NUL      SOH      STX      ETX      EOT      ENQ      ACK      BEL    */
	ch(0),   ch(1),   ch(2),   ch(3),   ch(4),   ch(5),   ch(6),   ch(7),
/*   BS       HT       LF       VT       FF       CR       SO       SI   */
	ch(8),   ch(9),   ch(10),  ch(11),  ch(12),  ch(13),  ch(14),  ch(15),

/*  DLE      DC1      DC2      DC3     DC4      NAK      SYN      ETB    */
	ch(16),  ch(17),  ch(18),  ch(19),  ch(20),  ch(21),  ch(22),  ch(23),
/* CAN      EM       SUB      ESC      FS       GS       RS       US     */
	ch(24),  ch(25),  ch(26),  ch(27),  ch(28),  ch(29),  ch(30),  ch(31),

/* SPACE      !        "        #        $        %        &        '    */
	ch(32),  ch(33),  ch(34),  ch(35),  ch(36),  ch(37),  ch(38),  ch(39),
/*   (        )        *        +        ,        -        .        /    */
	ch(40),  ch(41),  ch(42),  ch(43),  ch(44),  ch(45),  ch(46),  ch(47),

/*   0        1        2        3        4        5        6        7    */
	ch(130), ch(131), ch(132), ch(133), ch(134), ch(135), ch(136), ch(137),
/*   8        9        :        ;        <        =        >        ?    */
	ch(138), ch(139), ch(48),  ch(49),  ch(50),  ch(51),  ch(52),  ch(53),

/*   @        A        B        C        D        E        F        G    */
	ch(54),  ch(140), ch(142), ch(144), ch(146), ch(148), ch(150), ch(152),
/*   H        I        J        K        L        M        N        O    */
	ch(154), ch(156), ch(158), ch(160), ch(162), ch(164), ch(166), ch(170),

/*   P        Q        R        S        T        U        V        W    */
	ch(172), ch(174), ch(176), ch(178), ch(181), ch(183), ch(185), ch(187),
/*   X        Y        Z        [        \        ]        ^        _    */
	ch(189), ch(191), ch(196), ch(55),  ch(56),  ch(57),  ch(58),  ch(59),

/*   `        a        b        c        d        e        f        g    */
	ch(60),  ch(141), ch(143), ch(145), ch(147), ch(149), ch(151), ch(153),
/*   h        i        j        k        l        m        n        o    */
	ch(155), ch(157), ch(159), ch(161), ch(163), ch(165), ch(167), ch(171),

/*   p        q        r        s        t        u        v        w    */
	ch(173), ch(175), ch(177), ch(179), ch(182), ch(184), ch(186), ch(188),
/*   x        y        z        {        |        }        ~       DEL   */
	ch(190), ch(193), ch(197), ch(61),  ch(62),  ch(63),  ch(64),  ch(65),

/*   80       81       82       83       84       85       86       87   */
	ch(66),  ch(67),  ch(68),  ch(69),  ch(70),  ch(71),  ch(72),  ch(73),
/*   88       89       8A       8B       8C       8D       8E       8F   */
	ch(74),  ch(75),  ch(76),  ch(77),  ch(78),  ch(79),  ch(80),  ch(81),

/*   90       91       92       93       94       95       96       97   */
	ch(82),  ch(83),  ch(84),  ch(85),  ch(86),  ch(87),  ch(88),  ch(89),
/*   98       99       9A       9B       9C       9D       9E       9F   */
	ch(90),  ch(91),  ch(92),  ch(93),  ch(94),  ch(95),  ch(96),  ch(97),

/*   A0       A1       A2       A3       A4       A5       A6       A7   */
	ch(98),  ch(99),  ch(100), ch(101), ch(102), ch(103), ch(104), ch(105),
/*   A8       A9       AA       AB       AC       AD       AE       AF   */
	ch(106), ch(107), ch(108), ch(109), ch(110), ch(111), ch(112), ch(113),

/*   B0       B1       B2       B3       B4       B5       B6       B7   */
	ch(114), ch(115), ch(116), ch(117), ch(118), ch(119), ch(120), ch(121),
/*   B8       B9       BA       BB       BC       BD       BE       BF   */
	ch(122), ch(123), ch(124), ch(125), ch(126), ch(127), ch(128), ch(129),

/*   C0       C1       C2       C3       C4       C5       C6       C7   */
	ch(140), ch(140), ch(140), ch(140), ch(206), ch(204), ch(200), ch(144),
/*   C8       C9       CA       CB       CC       CD       CE       CF   */
	ch(148), ch(148), ch(148), ch(148), ch(156), ch(156), ch(156), ch(156),

/*   D0       D1       D2       D3       D4       D5       D6       D7   */
	ch(146), ch(168), ch(170), ch(170), ch(170), ch(170), ch(208), ch(170),
/*   D8       D9       DA       DB       DC       DD       DE       DF   */
	ch(202), ch(183), ch(183), ch(183), ch(183), ch(192), ch(198), ch(180),

/*   E0       E1       E2       E3       E4       E5       E6       E7   */
	ch(141), ch(141), ch(141), ch(141), ch(207), ch(205), ch(201), ch(145),
/*   E8       E9       EA       EB       EC       ED       EE       EF   */
	ch(149), ch(149), ch(149), ch(149), ch(157), ch(157), ch(157), ch(157),

/*   F0       F1       F2       F3       F4       F5       F6       F7   */
	ch(147), ch(169), ch(171), ch(171), ch(171), ch(171), ch(209), ch(171),
/*   F8       F9       FA       FB       FC       FD       FE       FF   */
	ch(203), ch(184), ch(184), ch(184), ch(184), ch(194), ch(199), ch(195)
	};

#undef ch

#endif /* DEBUG */
