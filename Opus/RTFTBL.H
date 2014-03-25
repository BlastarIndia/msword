/* ***************************************************************************
**
**      COPYRIGHT (C) 1987 MICROSOFT
**
** ***************************************************************************
*
*  Module: rtftbl.h - rtf keyword tables
*    these tables are generated or checked by rtfgen.c. They should be
*    kept in this order: rgszRtfSym, (iszRTF defines, imp defines,
*    irrb defines), rgrsymWord.
*
*    This file is intended to hold all the tables that need to be modified
*    when adding a new RTF keyword.
*
*
*
**
** REVISIONS
**
** Date         Who Rel Ver     Remarks
** 9/1/87       bz              split from rtf.c
**
** ************************************************************************ */


	/* these defines are unchanged and unchecked by rtfgen
		add an entry for any ChngDest entries, add external refs
		and an entry in rgpfnRtfActions to handle input for these
		kinds of entries.
	*/

		/* RTF destinations */
#define rdsMain        0
#define rdsPic         1
#define rdsFtn         2
#define rdsHdr         3
#define rdsStylesheet  4
#define rdsFonttbl     5
#define rdsColortbl    6
#define rdsComment     7
#define rdsPrivate1    8

#define rdsField       9
#define rdsFldInst     10
#define rdsFldRslt     11
#define rdsFldType     12

#define rdsInfoBlock   13    /* do not change order: InfoBlock->NofChars */
								/* used to index in rgifd in fieldspc.c */
#define rdsTitle       14    /* same order as flt codes */
#define rdsSubject     15
#define rdsAuthor      16
#define rdsKeywords    17
#define rdsDoccomm     18
#define rdsOperator    19
#define rdsCreatim     20
#define rdsRevtim      21
#define rdsPrintim     22

	/* these aren't really rds since the following do not make up a
		destination. However, they are numbers that should be
		contiguous with the previous set, so they are defined as
		rds's
	*/
#define rdsVersion     23
#define rdsEdmins      24
#define rdsNofpages    25
#define rdsNofwords    26
#define rdsNofchars    27   /* end of do-not-change-order */

#define rdsBuptim      28

#define rdsBkmkEnd     29
#define rdsBkmkStart   30

#define rdsAnnot       31
#define rdsAtnid       32
#define rdsTemplate    33
#define rdsGrid        34
#define rdsGridtbl     35
#define rdsXe          36
#define rdsTc          37
#define rdsTxe         38
#define rdsRxe         39

#ifdef RTFTABLES  /* all the rest of the file is under this flag */

	/* indexes into array of ChngDest routines */

#define ipfnRdsHdr              0
#define ipfnRdsPic              1
#define ipfnRdsPrivate1         2
#define ipfnRdsRtf              3
#define ipfnRdsStylesheet       4
#define ipfnDoParaEnd           5
#define ipfnDoSectEnd           6
#define ipfnQuote               7
#define ipfnTab                 8

#define ipfnRdsFld              9
#define ipfnRdsFldrslt          10
#define ipfnRdsFldIT            11
#define ipfnSpecChToFld         12
#define ipfnRdsNewStd           13
#define ipfnRdsGrid             14

#define ipfnDoTableRowEnd       15      /* emit end of table row mark */
#define ipfnTrnsfrmTxt          16      /* transform to different text */
#define ipfnTrnsfrmTcf          17      /* transform Tcf flag */
#define ipfnTrnsfrmTcl          18      /* transform Tcl flag */
#define ipfnRdsXeTc             19
#define ipfnRdsXeRT             20

	/*  array of ChngDest routines */

int     ChngToRdsHdr(), ChngToRdsPic(), ChngToRdsPrivate1(),
		ChngToRdsRtf(), ChngToRdsStylesheet(), DoRtfParaEnd(),
		DoRtfSectEnd(), DoQuoteForBinCode(), DoRtfTab();

int     ChngToRdsFld(), ChngToRdsFldRslt(), ChngToRdsFldIT(),
		SpecChToFld(), ChngToRdsNewStd(), ChngToRdsGrid();

int     DoRtfTableRowEnd();
int     TrnsfrmTxt(), TrnsfrmTcf(), TrnsfrmTcl();
int     ChngToRdsXeTc(), ChngToRdsXeRT();

csconst (*rgpfnRtfActions[])() =
		{
		ChngToRdsHdr, ChngToRdsPic, ChngToRdsPrivate1,
		ChngToRdsRtf, ChngToRdsStylesheet, DoRtfParaEnd,
		DoRtfSectEnd, DoQuoteForBinCode, DoRtfTab,
		ChngToRdsFld, ChngToRdsFldRslt, ChngToRdsFldIT,
		SpecChToFld,  ChngToRdsNewStd, ChngToRdsGrid,
		DoRtfTableRowEnd, TrnsfrmTxt, TrnsfrmTcf, 
		TrnsfrmTcl, ChngToRdsXeTc, ChngToRdsXeRT
		};

#define ibrcSingle 0
#define ibrcThick 1
#define ibrcTwoSingle 2
#define ibrcDotted 3
#define ibrcHairline 4

#ifdef RTFIN
csconst int rgwBrcBaseVals[] = {brcSingle, brcThick, brcTwoSingle, brcDotted, brcHairline};

csconst int rgicoNew [icoMax] =
	{icoAuto, icoBlack, icoBlue, icoGreen, icoCyan, icoRed, icoMagenta, 
			icoYellow, icoWhite}; 
#endif

		/* map dotted and hairline to single */
#define BrclFromIbrc(i)    ( ((unsigned) (i)) > ibrcTwoSingle ? brclSingle : \
								(i) )

/* *********************************************************** */
/* End unchanged table block */
/* *********************************************************** */




#ifdef DEBUG
#ifdef RTFSUBS  /* this only goes in rtfsubs.c */

csconst char rgszRtfSym[][] =

		/* Table of rtf keywords - must be sorted */

	{
	"\012",
	"\015",
	"'",
	"*",
	"-",
	":",
	"\134",
	"_",
	"absw",
	"annotation",
	"ansi",
	"atnid",
	"author",
	"b",
	"bin",
	"bkmkend",
	"bkmkstart",
	"blue",
	"box",
	"brdrb",
	"brdrbar",
	"brdrbtw",
	"brdrdb",
	"brdrdot",
	"brdrhair",
	"brdrl",
	"brdrr",
	"brdrs",
	"brdrsh",
	"brdrt",
	"brdrth",
	"brsp",

	"buptim",
	"bxe",
	"cell",
	"cellx",
	"cf",
	"chatn",
	"chdate",
	"chftn",
	"chftnsep",
	"chftnsepc",
	"chpgn",
	"chtime",
	"clbrdrb",
	"clbrdrl",
	"clbrdrr",
	"clbrdrt",
	"clmgf",
	"clmrg",
	"colortbl",
	"cols",
	"colsx",
	"column",
	"comment",
	"creatim",
	"date",
	"deff",
	"defformat",
	"deftab",
	"dn",
	"doccomm",
	"dxfrtext",
	"dy",
	"edmins",
	"enddoc",
	"endnhere",
	"endnotes",
	"expnd",
	"f",
	"facingp",
	"fdecor",
	"fi",

	"field",
	"flddirty",
	"fldedit",
	"fldinst",
	"fldlock",
	"fldpriv",
	"fldrslt",
	"fldtype",

	"fmodern",
	"fnil",
	"fonttbl",
	"footer",
	"footerf",
	"footerl",
	"footerr",
	"footery",
	"footnote",
	"fracwidth",
	"froman",
	"fs",
	"fscript",
	"fswiss",
	"ftech",
	"ftnbj",
	"ftncn",
	"ftnrestart",
	"ftnsep",
	"ftnsepc",
	"ftnstart",
	"ftntj",
	"g",
	"gcw",
	"green",
	"gridtbl",
	"gutter",
	"header",
	"headerf",
	"headerl",
	"headerr",
	"headery",
	"hr",
	"hyphhotz",
	"i",
	"id",
	"info",
	"intbl",
	"ixe",
	"keep",
	"keepn",
	"keywords",
	"landscape",
	"li",
	"line",
	"linebetcol",
	"linecont",
	"linemod",
	"lineppage",
	"linerestart",
	"linestart",
	"linestarts",
	"linex",
	"mac",
	"macpict",
	"makebackup",
	"margb",
	"margl",
	"margmirror",
	"margr",
	"margt",
	"min",
	"mo",
	"nextfile",
	"nofchars",
	"nofpages",
	"nofwords",
	"noline",
	"operator",
	"outl",
	"page",
	"pagebb",
	"paperh",
	"paperw",
	"par",
	"pard",
	"pc",
	"pca",
	"pgncont",
	"pgndec",
	"pgnlcltr",
	"pgnlcrm",
	"pgnrestart",
	"pgnstart",
	"pgnstarts",
	"pgnucltr",
	"pgnucrm",
	"pgnx",
	"pgny",
	"phcol",
	"phmrg",
	"phpg",

	"piccropb",
	"piccropl",
	"piccropr",
	"piccropt",
	"pich",
	"pichGoal",
	"picscaled",

	"picscalex",
	"picscaley",
	"pict",
	"picw",
	"picwGoal",

	"plain",

	"posx",
	"posxc",
	"posxi",
	"posxl",
	"posxo",
	"posxr",

	"posy",
	"posyb",
	"posyc",
	"posyil",
	"posyt",

	"printim",
	"private",
	"psover",
	"pvmrg",
	"pvpg",
	"qc",
	"qj",
	"ql",
	"qr",
	"red",
	"revbar",
	"revised",
	"revisions",
	"revprop",
	"revtim",
	"ri",
	"row",
	"rtf",
	"rxe",
	"s",
	"sa",
	"sb",
	"sbasedon",
	"sbkcol",
	"sbkeven",
	"sbknone",
	"sbkodd",
	"sbkpage",
	"sbys",
	"scaps",
	"sec",
	"sect",
	"sectd",
	"shad",

	"sl",
	"snext",
	"strike",
	"stylesheet",
	"subject",
	"tab",
	"tb",
	"tc",
	"tcelld",
	"tcf",
	"tcl",
	"template",
	"time",
	"title",
	"titlepg",
	"tldot",
	"tlhyph",
	"tlth",
	"tlul",
	"tqc",
	"tqdec",
	"tqr",
	"trgaph",
	"trleft",
	"trowd",
	"trqc",
	"trql",
	"trqr",
	"trrh",
	"tx",
	"txe",
	"ul",
	"uld",
	"uldb",
	"ulnone",
	"ulw",
	"up",
	"v",
	"vern",
	"version",
	"vertal",
	"vertalc",
	"vertalj",
	"vertalt",
	"wbitmap",
	"wbmbitspixel",
	"wbmplanes",
	"wbmwidthbytes",
	"widowctrl",
	"wmetafile",
	"xe",
	"yr",
	"{",
	"|",
	"}",
	"~"
	};

#endif /* RTFSUBS */

#endif /* DEBUG */


/* id's corresponding to keywords defined above. Must be kept in sync! */
/* Generate these #'s with rtfgen, which reads and remakes rtftbl.h */

#define iszRTFLF  	0
#define iszRTFCR  	1
#define iszRTFQuote  	2
#define iszRTFAsterisk  	3
#define iszRTFHyphen  	4
#define iszRTFColon  	5
#define iszRTFBackslash  	6
#define iszRTFUnderscore  	7
#define iszRTFabsw  	8
#define iszRTFannotation  	9
#define iszRTFansi  	10
#define iszRTFatnid  	11
#define iszRTFauthor  	12
#define iszRTFb  	13
#define iszRTFbin  	14
#define iszRTFbkmkend  	15
#define iszRTFbkmkstart  	16
#define iszRTFblue  	17
#define iszRTFbox  	18
#define iszRTFbrdrb  	19
#define iszRTFbrdrbar  	20
#define iszRTFbrdrbtw  	21
#define iszRTFbrdrdb  	22
#define iszRTFbrdrdot  	23
#define iszRTFbrdrhair  	24
#define iszRTFbrdrl  	25
#define iszRTFbrdrr  	26
#define iszRTFbrdrs  	27
#define iszRTFbrdrsh  	28
#define iszRTFbrdrt  	29
#define iszRTFbrdrth  	30
#define iszRTFbrsp  	31
#define iszRTFbuptim  	32
#define iszRTFbxe  	33
#define iszRTFcell  	34
#define iszRTFcellx  	35
#define iszRTFcf  	36
#define iszRTFchatn  	37
#define iszRTFchdate  	38
#define iszRTFchftn  	39
#define iszRTFchftnsep  	40
#define iszRTFchftnsepc  	41
#define iszRTFchpgn  	42
#define iszRTFchtime  	43
#define iszRTFclbrdrb  	44
#define iszRTFclbrdrl  	45
#define iszRTFclbrdrr  	46
#define iszRTFclbrdrt  	47
#define iszRTFclmgf  	48
#define iszRTFclmrg  	49
#define iszRTFcolortbl  	50
#define iszRTFcols  	51
#define iszRTFcolsx  	52
#define iszRTFcolumn  	53
#define iszRTFcomment  	54
#define iszRTFcreatim  	55
#define iszRTFdate  	56
#define iszRTFdeff  	57
#define iszRTFdefformat  	58
#define iszRTFdeftab  	59
#define iszRTFdn  	60
#define iszRTFdoccomm  	61
#define iszRTFdxfrtext  	62
#define iszRTFdy  	63
#define iszRTFedmins  	64
#define iszRTFenddoc  	65
#define iszRTFendnhere  	66
#define iszRTFendnotes  	67
#define iszRTFexpnd  	68
#define iszRTFf  	69
#define iszRTFfacingp  	70
#define iszRTFfdecor  	71
#define iszRTFfi  	72
#define iszRTFfield  	73
#define iszRTFflddirty  	74
#define iszRTFfldedit  	75
#define iszRTFfldinst  	76
#define iszRTFfldlock  	77
#define iszRTFfldpriv  	78
#define iszRTFfldrslt  	79
#define iszRTFfldtype  	80
#define iszRTFfmodern  	81
#define iszRTFfnil  	82
#define iszRTFfonttbl  	83
#define iszRTFfooter  	84
#define iszRTFfooterf  	85
#define iszRTFfooterl  	86
#define iszRTFfooterr  	87
#define iszRTFfootery  	88
#define iszRTFfootnote  	89
#define iszRTFfracwidth  	90
#define iszRTFfroman  	91
#define iszRTFfs  	92
#define iszRTFfscript  	93
#define iszRTFfswiss  	94
#define iszRTFftech  	95
#define iszRTFftnbj  	96
#define iszRTFftncn  	97
#define iszRTFftnrestart  	98
#define iszRTFftnsep  	99
#define iszRTFftnsepc  	100
#define iszRTFftnstart  	101
#define iszRTFftntj  	102
#define iszRTFg  	103
#define iszRTFgcw  	104
#define iszRTFgreen  	105
#define iszRTFgridtbl  	106
#define iszRTFgutter  	107
#define iszRTFheader  	108
#define iszRTFheaderf  	109
#define iszRTFheaderl  	110
#define iszRTFheaderr  	111
#define iszRTFheadery  	112
#define iszRTFhr  	113
#define iszRTFhyphhotz  	114
#define iszRTFi  	115
#define iszRTFid  	116
#define iszRTFinfo  	117
#define iszRTFintbl  	118
#define iszRTFixe  	119
#define iszRTFkeep  	120
#define iszRTFkeepn  	121
#define iszRTFkeywords  	122
#define iszRTFlandscape  	123
#define iszRTFli  	124
#define iszRTFline  	125
#define iszRTFlinebetcol  	126
#define iszRTFlinecont  	127
#define iszRTFlinemod  	128
#define iszRTFlineppage  	129
#define iszRTFlinerestart  	130
#define iszRTFlinestart  	131
#define iszRTFlinestarts  	132
#define iszRTFlinex  	133
#define iszRTFmac  	134
#define iszRTFmacpict  	135
#define iszRTFmakebackup  	136
#define iszRTFmargb  	137
#define iszRTFmargl  	138
#define iszRTFmargmirror  	139
#define iszRTFmargr  	140
#define iszRTFmargt  	141
#define iszRTFmin  	142
#define iszRTFmo  	143
#define iszRTFnextfile  	144
#define iszRTFnofchars  	145
#define iszRTFnofpages  	146
#define iszRTFnofwords  	147
#define iszRTFnoline  	148
#define iszRTFoperator  	149
#define iszRTFoutl  	150
#define iszRTFpage  	151
#define iszRTFpagebb  	152
#define iszRTFpaperh  	153
#define iszRTFpaperw  	154
#define iszRTFpar  	155
#define iszRTFpard  	156
#define iszRTFpc  	157
#define iszRTFpca  	158
#define iszRTFpgncont  	159
#define iszRTFpgndec  	160
#define iszRTFpgnlcltr  	161
#define iszRTFpgnlcrm  	162
#define iszRTFpgnrestart  	163
#define iszRTFpgnstart  	164
#define iszRTFpgnstarts  	165
#define iszRTFpgnucltr  	166
#define iszRTFpgnucrm  	167
#define iszRTFpgnx  	168
#define iszRTFpgny  	169
#define iszRTFphcol  	170
#define iszRTFphmrg  	171
#define iszRTFphpg  	172
#define iszRTFpiccropb  	173
#define iszRTFpiccropl  	174
#define iszRTFpiccropr  	175
#define iszRTFpiccropt  	176
#define iszRTFpich  	177
#define iszRTFpichGoal  	178
#define iszRTFpicscaled  	179
#define iszRTFpicscalex  	180
#define iszRTFpicscaley  	181
#define iszRTFpict  	182
#define iszRTFpicw  	183
#define iszRTFpicwGoal  	184
#define iszRTFplain  	185
#define iszRTFposx  	186
#define iszRTFposxc  	187
#define iszRTFposxi  	188
#define iszRTFposxl  	189
#define iszRTFposxo  	190
#define iszRTFposxr  	191
#define iszRTFposy  	192
#define iszRTFposyb  	193
#define iszRTFposyc  	194
#define iszRTFposyil  	195
#define iszRTFposyt  	196
#define iszRTFprintim  	197
#define iszRTFprivate  	198
#define iszRTFpsover  	199
#define iszRTFpvmrg  	200
#define iszRTFpvpg  	201
#define iszRTFqc  	202
#define iszRTFqj  	203
#define iszRTFql  	204
#define iszRTFqr  	205
#define iszRTFred  	206
#define iszRTFrevbar  	207
#define iszRTFrevised  	208
#define iszRTFrevisions  	209
#define iszRTFrevprop  	210
#define iszRTFrevtim  	211
#define iszRTFri  	212
#define iszRTFrow  	213
#define iszRTFrtf  	214
#define iszRTFrxe  	215
#define iszRTFs  	216
#define iszRTFsa  	217
#define iszRTFsb  	218
#define iszRTFsbasedon  	219
#define iszRTFsbkcol  	220
#define iszRTFsbkeven  	221
#define iszRTFsbknone  	222
#define iszRTFsbkodd  	223
#define iszRTFsbkpage  	224
#define iszRTFsbys  	225
#define iszRTFscaps  	226
#define iszRTFsec  	227
#define iszRTFsect  	228
#define iszRTFsectd  	229
#define iszRTFshad  	230
#define iszRTFsl  	231
#define iszRTFsnext  	232
#define iszRTFstrike  	233
#define iszRTFstylesheet  	234
#define iszRTFsubject  	235
#define iszRTFtab  	236
#define iszRTFtb  	237
#define iszRTFtc  	238
#define iszRTFtcelld  	239
#define iszRTFtcf  	240
#define iszRTFtcl  	241
#define iszRTFtemplate  	242
#define iszRTFtime  	243
#define iszRTFtitle  	244
#define iszRTFtitlepg  	245
#define iszRTFtldot  	246
#define iszRTFtlhyph  	247
#define iszRTFtlth  	248
#define iszRTFtlul  	249
#define iszRTFtqc  	250
#define iszRTFtqdec  	251
#define iszRTFtqr  	252
#define iszRTFtrgaph  	253
#define iszRTFtrleft  	254
#define iszRTFtrowd  	255
#define iszRTFtrqc  	256
#define iszRTFtrql  	257
#define iszRTFtrqr  	258
#define iszRTFtrrh  	259
#define iszRTFtx  	260
#define iszRTFtxe  	261
#define iszRTFul  	262
#define iszRTFuld  	263
#define iszRTFuldb  	264
#define iszRTFulnone  	265
#define iszRTFulw  	266
#define iszRTFup  	267
#define iszRTFv  	268
#define iszRTFvern  	269
#define iszRTFversion  	270
#define iszRTFvertal  	271
#define iszRTFvertalc  	272
#define iszRTFvertalj  	273
#define iszRTFvertalt  	274
#define iszRTFwbitmap  	275
#define iszRTFwbmbitspixel  	276
#define iszRTFwbmplanes  	277
#define iszRTFwbmwidthbytes  	278
#define iszRTFwidowctrl  	279
#define iszRTFwmetafile  	280
#define iszRTFxe  	281
#define iszRTFyr  	282
#define iszRTFLBracket  	283
#define iszRTFFormula  	284
#define iszRTFRBracket  	285
#define iszRTFTilde  	286

#define iszRTFMax	287



/* Rtf Record Block indexes - match properties to structures.

   the numbers for these defines are generated by rtfgen.c. It takes an 
   existing table and starts at 0 and increments on each define except:
   following irrb...First or irrb...Lim or irrb...Max... we don't increment,
   and the value for irrb...First is not incremented.
*/

#define irrbPapFirst  	0
#define irrbRs  	0
#define irrbRjc  	1
#define irrbRfi  	2
#define irrbRli  	3
#define irrbRri  	4
#define irrbRsb  	5
#define irrbRsa  	6
#define irrbRsl  	7
#define irrbRkeep  	8
#define irrbRkeepn  	9
#define irrbRpagebb  	10
#define irrbRnoline  	11
#define irrbRintbl  	12
#define irrbRpbrc  	13
#define irrbRpcVert  	14
#define irrbRpcHorz  	15
#define irrbRdxaAbs  	16
#define irrbRdyaAbs  	17
#define irrbRabsw  	18
#define irrbRdxfrtext  	19
#define irrbRtx  	20

#define irrbPapLim  	21

#define irrbRtlc  	21
#define irrbRtjc  	22
#define irrbRtb  	23
#define irrbRdxaAbsNeg  	24
#define irrbRdyaAbsNeg  	25
#define irrbRpard  	26

#define irrbMaxPap  	27

#define irrbChpFirst  	27
#define irrbRb  	27
#define irrbRi  	28
#define irrbRstrike  	29
#define irrbRoutl  	30
#define irrbRscaps  	31
#define irrbRv  	32
#define irrbRf  	33
#define irrbRfs  	34
#define irrbRkul  	35
#define irrbRexpnd  	36
#define irrbRcf  	37
#define irrbRup  	38
#define irrbRrevised  	39

#define irrbChpLim  	40

				/* up, down are really the same in the chp */
#define irrbRdn  	40
#define irrbRplain  	41

#define irrbMaxChp  	42

#define irrbDopFirst  	42
#define irrbRpaperw  	42
#define irrbRpaperh  	43
#define irrbRmargl  	44
#define irrbRmargr  	45
#define irrbRmargt  	46
#define irrbRmargb  	47
#define irrbRfacingp  	48
#define irrbRgutter  	49
#define irrbRdeftab  	50
#define irrbRwidowctrl  	51
#define irrbRfpc  	52
#define irrbRftnstart  	53
#define irrbRftnrestart  	54
#define irrbRpgnstart  	55
#define irrbRlinestart  	56
#define irrbRlandscape  	57
#define irrbRhyphhotz  	58
#define irrbRrevisions  	59
#define irrbRrevbar  	60
#define irrbRrevprop  	61
#define irrbRmakebackup  	62
#define irrbRmargmirror  	63

#define irrbDopLim  	64
#define irrbMaxDop  	64

#define irrbSepFirst  	64
#define irrbRbkc  	64
#define irrbRpgc  	65
#define irrbRpgnstarts  	66
#define irrbRnfc  	67
#define irrbRlinemod  	68
#define irrbRlinex  	69
#define irrbRlnc  	70
#define irrbRlinestarts  	71
#define irrbRheadery  	72
#define irrbRfootery  	73
#define irrbRcols  	74
#define irrbRcolsx  	75
#define irrbRlinebetcol  	76
#define irrbRvjc  	77
#define irrbRendnhere  	78
#define irrbRtitlepg  	79

#define irrbSepLim  	80

#define irrbRsectd  	80

#define irrbMaxSep  	81

#define irrbRsbys  	81
#define irrbRpgnx  	82
#define irrbRpgny  	83

#define irrbRpiccropb  	84
#define irrbRpiccropl  	85
#define irrbRpiccropr  	86
#define irrbRpiccropt  	87
#define irrbRpicscalex  	88
#define irrbRpicscaley  	89
#define irrbRpich  	90
#define irrbRpichGoal  	91
#define irrbRpicw  	92
#define irrbRpicwGoal  	93
#define irrbRwbmbitspixel  	94
#define irrbRwbmplanes  	95
#define irrbRwbmwidthbytes  	96
#define irrbRmacpict  	97
#define irrbRwbitmap  	98
#define irrbRwmetafile  	99

#define irrbRchs  	100
#define irrbRfmc  	101

#define irrbRblue  	102
#define irrbRgreen  	103
#define irrbRred  	104

#define irrbRsbasedon  	105
#define irrbRsnext  	106

#define irrbRflddirty  	107
#define irrbRfldedit  	108
#define irrbRfldlock  	109
#define irrbRfldpriv  	110

#define irrbRBrcFirst  	111

#define irrbRbrdrsh  	111
#define irrbRbrsp  	112
#define irrbRbrcBase  	113

#define irrbRBrcLim  	114
#define irrbRMaxBrc  	114

#define irrbRTapFirst  	114

#define irrbRtcjc  	114
#define irrbRtrgaph  	115
#define irrbRtrrh  	116
#define irrbRtrleft  	117

#define irrbRTapLim  	118
#define irrbRTcFirst  	118

#define irrbRclmgf  	118
#define irrbRclmrg  	119
#define irrbRclbrc  	120
#define irrbRTcLim  	121

#define irrbRcellx  	121
#define irrbRtrowd  	122
#define irrbRtcelld  	123

#define irrbRMaxTc  	124


#define irrbRdy  	124
#define irrbRhr  	125
#define irrbRmin  	126
#define irrbRmo  	127
#define irrbRsec  	128
#define irrbRyr  	129

#define irrbRversion  	130
#define irrbRnofchars  	131
#define irrbRnofpages  	132
#define irrbRnofwords  	133
#define irrbRedmins  	134

#define irrbRg  	135
#define irrbRgcw  	136

#define irrbRAsterisk  	137
#define irrbRdeff  	138

#define irrbRvern  	139

#define irrbRIgnore  	140

#define irrbRpicscaled  irrbRIgnore    /* unused in Opus */
#define irrbRshad       irrbRIgnore    /* unused in Opus */
#define irrbRdefformat  irrbRIgnore    /* unused in Opus */
#define irrbRfracwidth  irrbRIgnore    /* unused in Opus */
#define irrbRpsover     irrbRIgnore    /* unused in Opus */

#define irrbRnextfile   irrbRIgnore
#define irrbRid         irrbRIgnore

		/* these entries are in the table for use in converting special
			characters to fields and are not used in normal processing.
			It is just a handy place to put the converting string */

#define irrbRdate       irrbRIgnore
#define irrbRtime       irrbRIgnore
#define irrbRl          irrbRIgnore


#ifdef RTFIN
csconst struct RSYM rgrsymWord[] =

/* RtfIn symbol table. Must be in same order as keyword names. */

/*                      val,              fPassVal, rac,          irrb/ipfn/fSpec */
	{
/* iszRTFLF         */ {chEop,            fTrue,  racSpecCharAct, ipfnDoParaEnd},
/* iszRTFCR         */ {chEop,            fTrue,  racSpecCharAct, ipfnDoParaEnd},
/* iszRTFQuote      */ {0,                fFalse, racSpecCharAct, ipfnQuote },
/* iszRTFAsterisk   */ {0,                fFalse, racChngProp,    irrbRAsterisk},
/* iszRTFHyphen     */ {chNonReqHyphen,   fTrue,  racSpecChar,    0 },
/* iszRTFColon      */ {chColon,          fTrue,  racSpecCharAct, ipfnTrnsfrmTxt},
/* iszRTFBackslash  */ {chBackslash,      fTrue,  racSpecChar,    0 },
/* iszRTFUnderscore */ {chNonBreakHyphen, fTrue,  racSpecChar,    0 },
/* iszRTFabsw       */ {0,                fFalse, racChngProp,    irrbRabsw},
/* iszRTFannotation */ {rdsAnnot,         fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFansi       */ {chsAnsi,          fTrue,  racChngProp,    irrbRchs},
/* iszRTFatnid      */ {rdsAtnid,         fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFauthor     */ {rdsAuthor,        fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFb          */ {0,                fFalse, racChngProp,    irrbRb},
/* iszRTFbin        */ {0,                fFalse, racBinParm,     0},
/* iszRTFbkmkend    */ {rdsBkmkEnd,       fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFbkmkstart  */ {rdsBkmkStart,     fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFblue       */ {0,                fFalse, racChngProp,    irrbRblue},
/* iszRTFbox        */ {6,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrb      */ {2,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrbar    */ {5,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrbtw    */ {4,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrdb     */ {ibrcTwoSingle,    fTrue,  racChngProp,    irrbRbrcBase},
/* iszRTFbrdrdot    */ {ibrcDotted,       fTrue,  racChngProp,    irrbRbrcBase},
/* iszRTFbrdrhair   */ {ibrcHairline,     fTrue,  racChngProp,    irrbRbrcBase},
/* iszRTFbrdrl      */ {1,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrr      */ {3,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrs      */ {ibrcSingle,       fTrue,  racChngProp,    irrbRbrcBase},
/* iszRTFbrdrsh     */ {0,                fFalse, racChngProp,    irrbRbrdrsh},
/* iszRTFbrdrt      */ {0,                fTrue,  racChngProp,    irrbRpbrc},
/* iszRTFbrdrth     */ {ibrcThick,        fTrue,  racChngProp,    irrbRbrcBase},
/* iszRTFbrsp       */ {0,                fFalse, racChngProp,    irrbRbrsp},
/* iszRTFbuptim     */ {rdsBuptim,        fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFbxe        */ {chFldXeBold,      fTrue,  racSpecCharAct, ipfnTrnsfrmTxt},
/* iszRTFcell       */ {chTable,          fTrue,  racSpecCharAct, ipfnDoParaEnd},
/* iszRTFcellx      */ {0,                fFalse, racChngProp,    irrbRcellx},
/* iszRTFcf         */ {0,                fFalse, racChngProp,    irrbRcf},
/* iszRTFchatn      */ {chAtn,            fTrue,  racSpecChar,    fTrue},
/* iszRTFchdate     */ {iszRTFdate,       fTrue,  racSpecCharAct, ipfnSpecChToFld},
/* iszRTFchftn      */ {chFootnote,       fTrue,  racSpecChar,    fTrue},
/* iszRTFchftnsep   */ {chTFtn,           fTrue,  racSpecChar,    fTrue},
/* iszRTFchftnsepc  */ {chTFtnCont,       fTrue,  racSpecChar,    fTrue},
/* iszRTFchpgn      */ {iszRTFpage,       fTrue,  racSpecCharAct, ipfnSpecChToFld},
/* iszRTFchtime     */ {iszRTFtime,       fTrue,  racSpecCharAct, ipfnSpecChToFld},
/* iszRTFclbrdrb    */ {2,            fTrue,  racChngProp,    irrbRclbrc},
/* iszRTFclbrdrl    */ {1,            fTrue,  racChngProp,    irrbRclbrc},
/* iszRTFclbrdrr    */ {3,            fTrue,  racChngProp,    irrbRclbrc},
/* iszRTFclbrdrt    */ {0,            fTrue,  racChngProp,    irrbRclbrc},
/* iszRTFclmgf      */ {0,            fFalse, racChngProp,    irrbRclmgf},
/* iszRTFclmrg      */ {0,            fFalse, racChngProp,    irrbRclmrg},
/* iszRTFcolortbl   */ {rdsColortbl,      fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFcols       */ {0,                fFalse, racChngProp,    irrbRcols},
/* iszRTFcolsx      */ {0,                fFalse, racChngProp,    irrbRcolsx},
/* iszRTFcolumn     */ {chColumnBreak,    fTrue,  racSpecChar,    fFalse},
/* iszRTFcomment    */ {rdsComment,       fTrue,  racScanByDest,  ipfnRdsNewStd},
/* iszRTFcreatim    */ {rdsCreatim,       fTrue,  racChngDest,    ipfnRdsNewStd},
		/* this is really a dummy entry */
/* iszRTFdate       */ {0,                fFalse, racChngProp,    irrbRdate},
/* iszRTFdeff       */ {0,                fFalse,  racChngProp,    irrbRdeff},
/* iszRTFdefformat  */ {0,                fFalse, racChngProp,    irrbRdefformat},
/* iszRTFdeftab     */ {0,                fFalse, racChngProp,    irrbRdeftab},
/* iszRTFdn         */ {0,                fFalse, racChngProp,    irrbRdn},
/* iszRTFdoccomm    */ {rdsDoccomm,       fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFdxfrtext   */ {0,                fFalse, racChngProp,    irrbRdxfrtext},
/* iszRTFdy         */ {0,                fFalse, racChngProp,    irrbRdy},
/* iszRTFedmins     */ {0,                fFalse, racChngProp,    irrbRedmins},
/* iszRTFenddoc     */ {fpcEndDoc,        fTrue,  racChngProp,    irrbRfpc},
/* iszRTFendnhere   */ {0,                fFalse, racChngProp,    irrbRendnhere},
/* iszRTFendnotes   */ {fpcEndnote,       fTrue,  racChngProp,    irrbRfpc},
/* iszRTFexpnd      */ {0,                fFalse, racChngProp,    irrbRexpnd},
/* iszRTFf          */ {0,                fFalse, racChngProp,    irrbRf},
/* iszRTFfacingp    */ {0,                fFalse, racChngProp,    irrbRfacingp},
/* iszRTFfdecor     */ {fmcDecor,         fTrue,  racChngProp,    irrbRfmc},
/* iszRTFfi         */ {0,                fFalse, racChngProp,    irrbRfi},

/* iszRTFfield      */ {0,                fFalse, racChngDest,    ipfnRdsFld},
/* iszRTFflddirty   */ {fTrue,            fTrue,  racChngProp,    irrbRflddirty},
/* iszRTFfldedit    */ {fTrue,            fTrue,  racChngProp,    irrbRfldedit},
/* iszRTFfldinst    */ {rdsFldInst,       fTrue,  racChngDest,    ipfnRdsFldIT},
/* iszRTFfldlock    */ {fTrue,            fTrue,  racChngProp,    irrbRfldlock},
/* iszRTFfldpriv    */ {fTrue,            fTrue,  racChngProp,    irrbRfldpriv},
/* iszRTFfldrslt    */ {0,                fFalse, racChngDest,    ipfnRdsFldrslt},
/* iszRTFfldtype    */ {rdsFldType,       fTrue,  racChngDest,    ipfnRdsFldIT},

/* iszRTFfmodern    */ {fmcModern,        fTrue,  racChngProp,    irrbRfmc},
/* iszRTFfnil       */ {fmcNil,           fTrue,  racChngProp,    irrbRfmc},
/* iszRTFfonttbl    */ {rdsFonttbl,       fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFfooter     */ {ihdtBRight,       fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFfooterf    */ {ihdtBFirst,       fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFfooterl    */ {ihdtBLeft,        fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFfooterr    */ {ihdtBRight,       fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFfootery    */ {0,                fFalse, racChngProp,    irrbRfootery},
/* iszRTFfootnote   */ {rdsFtn,           fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFfracwidth  */ {0,                fFalse, racChngProp,    irrbRfracwidth},
/* iszRTFfroman     */ {fmcRoman,         fTrue,  racChngProp,    irrbRfmc},
/* iszRTFfs         */ {0,                fFalse, racChngProp,    irrbRfs},
/* iszRTFfscript    */ {fmcScript,        fTrue,  racChngProp,    irrbRfmc},
/* iszRTFfswiss     */ {fmcSwiss,         fTrue,  racChngProp,    irrbRfmc},
/* iszRTFftech      */ {fmcNil,           fTrue,  racChngProp,    irrbRfmc},
/* iszRTFftnbj      */ {fpcBottomPage,    fTrue,  racChngProp,    irrbRfpc},
/* iszRTFftncn      */ {ihdtBFtnCont,     fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFftnrestart */ {0,                fFalse, racChngProp,    irrbRftnrestart},
/* iszRTFftnsep     */ {ihdtTFtn,         fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFftnsepc    */ {ihdtTFtnCont,     fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFftnstart   */ {0,                fFalse, racChngProp,    irrbRftnstart},
/* iszRTFftntj      */ {fpcBeneathText,   fTrue,  racChngProp,    irrbRfpc},
/* iszRTFg          */ {0,                fFalse, racChngDest,    ipfnRdsGrid},
/* iszRTFgcw        */ {0,                fFalse, racChngProp,    irrbRgcw},
/* iszRTFgreen      */ {0,                fFalse, racChngProp,    irrbRgreen},
/* iszRTFgridtbl    */ {rdsGridtbl,       fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFgutter     */ {0,                fFalse, racChngProp,    irrbRgutter},
/* iszRTFheader     */ {ihdtTRight,       fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFheaderf    */ {ihdtTFirst,       fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFheaderl    */ {ihdtTLeft,        fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFheaderr    */ {ihdtTRight,       fTrue,  racChngDest,    ipfnRdsHdr},
/* iszRTFheadery    */ {0,                fFalse, racChngProp,    irrbRheadery},
/* iszRTFhr         */ {0,                fFalse, racChngProp,    irrbRhr},
/* iszRTFhyphhotz   */ {0,                fFalse, racChngProp,    irrbRhyphhotz},
/* iszRTFi          */ {0,                fFalse, racChngProp,    irrbRi},
/* iszRTFid         */ {0,                fFalse, racChngProp,    irrbRid},
/* iszRTFinfo       */ {rdsInfoBlock,     fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFintbl      */ {0,                fFalse, racChngProp,    irrbRintbl},
/* iszRTFixe        */ {chFldXeItalic,    fTrue,  racSpecCharAct, ipfnTrnsfrmTxt},
/* iszRTFkeep       */ {0,                fFalse, racChngProp,    irrbRkeep},
/* iszRTFkeepn      */ {0,                fFalse, racChngProp,    irrbRkeepn},
/* iszRTFkeywords   */ {rdsKeywords,      fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFlandscape  */ {0,                fFalse, racChngProp,    irrbRlandscape},
/* iszRTFli         */ {0,                fFalse, racChngProp,    irrbRli},
/* iszRTFline       */ {chCRJ,            fTrue,  racSpecChar,    fFalse},
/* iszRTFlinebetcol */ {0,                fFalse, racChngProp,    irrbRlinebetcol},
/* iszRTFlinecont   */ {lncContinue,      fTrue,  racChngProp,    irrbRlnc},
/* iszRTFlinemod    */ {0,                fFalse, racChngProp,    irrbRlinemod},
/* iszRTFlineppage  */ {lncPerPage,       fTrue,  racChngProp,    irrbRlnc},
/* iszRTFlinerestart*/ {lncRestart,       fTrue,  racChngProp,    irrbRlnc},
/* iszRTFlinestart  */ {0,                fFalse, racChngProp,    irrbRlinestart},
/* iszRTFlinestarts */ {0,                fFalse, racChngProp,    irrbRlinestarts},
/* iszRTFlinex      */ {0,                fFalse, racChngProp,    irrbRlinex},
/* iszRTFmac        */ {chsMac,           fTrue,  racChngProp,    irrbRchs},
/* iszRTFmacpict    */ {0,                fFalse, racChngProp,    irrbRmacpict},
/* iszRTFmakebackup */ {0,                fFalse, racChngProp,    irrbRmakebackup},
/* iszRTFmargb      */ {0,                fFalse, racChngProp,    irrbRmargb},
/* iszRTFmargl      */ {0,                fFalse, racChngProp,    irrbRmargl},
/* iszRTFmargmirror */ {0,                fFalse, racChngProp,    irrbRmargmirror},
/* iszRTFmargr      */ {0,                fFalse, racChngProp,    irrbRmargr},
/* iszRTFmargt      */ {0,                fFalse, racChngProp,    irrbRmargt},
/* iszRTFmin        */ {0,                fFalse, racChngProp,    irrbRmin},
/* iszRTFmo         */ {0,                fFalse, racChngProp,    irrbRmo},
/* iszRTFnextfile   */ {0,                fFalse, racChngProp,    irrbRnextfile},
/* iszRTFnofchars   */ {0,                fFalse, racChngProp,    irrbRnofchars},
/* iszRTFnofpages   */ {0,                fFalse, racChngProp,    irrbRnofpages},
/* iszRTFnofwords   */ {0,                fFalse, racChngProp,    irrbRnofwords},
/* iszRTFnoline     */ {0,                fFalse, racChngProp,    irrbRnoline},
/* iszRTFoperator   */ {rdsOperator,      fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFoutl       */ {0,                fFalse, racChngProp,    irrbRoutl},
/* iszRTFpage       */ {chSect,           fTrue,  racSpecChar,    fFalse},
/* iszRTFpagebb     */ {0,                fFalse, racChngProp,    irrbRpagebb},
/* iszRTFpaperh     */ {0,                fFalse, racChngProp,    irrbRpaperh},
/* iszRTFpaperw     */ {0,                fFalse, racChngProp,    irrbRpaperw},
/* iszRTFpar        */ {chEop,            fTrue,  racSpecCharAct, ipfnDoParaEnd},
/* iszRTFpard       */ {0,                fFalse, racChngProp,    irrbRpard},
/* iszRTFpc         */ {chsPC,            fTrue,  racChngProp,    irrbRchs},
/* iszRTFpca        */ {chsPCA,           fTrue,  racChngProp,    irrbRchs},
/* iszRTFpgncont    */ {fFalse,           fTrue,  racChngProp,    irrbRpgc},
/* iszRTFpgndec     */ {nfcArabic,        fTrue,  racChngProp,    irrbRnfc},
/* iszRTFpgnlcltr   */ {nfcLCLetter,      fTrue,  racChngProp,    irrbRnfc},
/* iszRTFpgnlcrm    */ {nfcLCRoman,       fTrue,  racChngProp,    irrbRnfc},
/* iszRTFpgnrestart */ {fTrue,            fTrue,  racChngProp,    irrbRpgc},
/* iszRTFpgnstart   */ {0,                fFalse, racChngProp,    irrbRpgnstart},
/* iszRTFpgnstarts  */ {0,                fFalse, racChngProp,    irrbRpgnstarts},
/* iszRTFpgnucltr   */ {nfcUCLetter,      fTrue,  racChngProp,    irrbRnfc},
/* iszRTFpgnucrm    */ {nfcUCRoman,       fTrue,  racChngProp,    irrbRnfc},
/* iszRTFpgnx       */ {0,                fFalse, racChngProp,    irrbRpgnx},
/* iszRTFpgny       */ {0,                fFalse, racChngProp,    irrbRpgny},

/* iszRTFphcol      */ {pcHColumn,        fTrue,  racChngProp,    irrbRpcHorz},
/* iszRTFphmrg      */ {pcHMargin,        fTrue,  racChngProp,    irrbRpcHorz},
/* iszRTFphpg       */ {pcHPage,          fTrue,  racChngProp,    irrbRpcHorz},

/* iszRTFpiccropb      */ {0,         fFalse, racChngProp,    irrbRpiccropb},
/* iszRTFpiccropl      */ {0,         fFalse, racChngProp,    irrbRpiccropl},
/* iszRTFpiccropr      */ {0,         fFalse, racChngProp,    irrbRpiccropr},
/* iszRTFpiccropt      */ {0,         fFalse, racChngProp,    irrbRpiccropt},

/* iszRTFpich          */ {0,         fFalse, racChngProp,    irrbRpich},
/* iszRTFpichGoal      */ {0,         fFalse, racChngProp,    irrbRpichGoal},

/* iszRTFpicscaled  */ {0,         fFalse, racChngProp,    irrbRpicscaled},

/* iszRTFpicscalex  */ {0,         fFalse, racChngProp,    irrbRpicscalex},
/* iszRTFpicscaley  */ {0,         fFalse, racChngProp,    irrbRpicscaley},
/* iszRTFpict       */ {0,         fFalse, racChngDest,    ipfnRdsPic},
/* iszRTFpicw       */ {0,         fFalse, racChngProp,    irrbRpicw},
/* iszRTFpicwGoal   */ {0,         fFalse, racChngProp,    irrbRpicwGoal},

/* iszRTFplain      */ {0,         fFalse, racChngProp,    irrbRplain},

/* iszRTFposx       */ {0,               fFalse, racChngProp,    irrbRdxaAbs},
/* iszRTFposxc      */ {-dxaAbsCenter,   fTrue,  racChngProp,    irrbRdxaAbsNeg},
/* iszRTFposxi      */ {-dxaAbsInside,   fTrue,  racChngProp,    irrbRdxaAbsNeg},
/* iszRTFposxl      */ {-dxaAbsLeft,     fTrue,  racChngProp,    irrbRdxaAbsNeg},
/* iszRTFposxo      */ {-dxaAbsOutside,  fTrue,  racChngProp,    irrbRdxaAbsNeg},
/* iszRTFposxr      */ {-dxaAbsRight,    fTrue,  racChngProp,    irrbRdxaAbsNeg},

/* iszRTFposy       */ {0,               fFalse, racChngProp,    irrbRdyaAbs},
/* iszRTFposyb      */ {-dyaAbsBottom,   fTrue,  racChngProp,    irrbRdyaAbsNeg},
/* iszRTFposyc      */ {-dyaAbsCenter,   fTrue,  racChngProp,    irrbRdyaAbsNeg},
/* iszRTFposyil     */ {-dyaAbsInline,   fTrue,  racChngProp,    irrbRdyaAbsNeg},
/* iszRTFposyt      */ {-dyaAbsTop,      fTrue,  racChngProp,    irrbRdyaAbsNeg},

/* iszRTFprintim    */ {rdsPrintim,       fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFprivate    */ {0,                fFalse, racChngDest,    ipfnRdsPrivate1},
/* iszRTFpsover     */ {0,                fFalse, racChngProp,    irrbRpsover},
/* iszRTFpvmrg      */ {pcVMargin,        fTrue,  racChngProp,    irrbRpcVert},
/* iszRTFpvpg       */ {pcVPage,          fTrue,  racChngProp,    irrbRpcVert},
/* iszRTFqc         */ {jcCenter,         fTrue,  racChngProp,    irrbRjc},
/* iszRTFqj         */ {jcBoth,           fTrue,  racChngProp,    irrbRjc},
/* iszRTFql         */ {jcLeft,           fTrue,  racChngProp,    irrbRjc},
/* iszRTFqr         */ {jcRight,          fTrue,  racChngProp,    irrbRjc},
/* iszRTFred        */ {0,                fFalse, racChngProp,    irrbRred},
/* iszRTFrevbar     */ {0,                fFalse, racChngProp,    irrbRrevbar},
/* iszRTFrevised    */ {0,                fFalse, racChngProp,    irrbRrevised},
/* iszRTFrevisions  */ {0,                fFalse, racChngProp,    irrbRrevisions},
/* iszRTFrevprop    */ {0,                fFalse, racChngProp,    irrbRrevprop},
/* iszRTFrevtim     */ {rdsRevtim,        fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFri         */ {0,                fFalse, racChngProp,    irrbRri},
/* iszRTFrow        */ {chTable,          fTrue,  racSpecCharAct, ipfnDoTableRowEnd},
/* iszRTFrtf        */ {0,                fFalse, racChngDest,    ipfnRdsRtf},
/* iszRTFrxe        */ {rdsRxe,           fTrue,  racChngDest,    ipfnRdsXeRT},
/* iszRTFs          */ {0,                fFalse, racChngProp,    irrbRs},
/* iszRTFsa         */ {0,                fFalse, racChngProp,    irrbRsa},
/* iszRTFsb         */ {0,                fFalse, racChngProp,    irrbRsb},
/* iszRTFsbasedon   */ {0,                fFalse, racChngProp,    irrbRsbasedon},
/* iszRTFsbkcol     */ {bkcNewColumn,     fTrue,  racChngProp,    irrbRbkc},
/* iszRTFsbkeven    */ {bkcEvenPage,      fTrue,  racChngProp,    irrbRbkc},
/* iszRTFsbknone    */ {bkcNoBreak,       fTrue,  racChngProp,    irrbRbkc},
/* iszRTFsbkodd     */ {bkcOddPage,       fTrue,  racChngProp,    irrbRbkc},
/* iszRTFsbkpage    */ {bkcNewPage,       fTrue,  racChngProp,    irrbRbkc},
/* iszRTFsbys       */ {0,                fFalse, racChngProp,    irrbRsbys},
/* iszRTFscaps      */ {0,                fFalse, racChngProp,    irrbRscaps},
/* iszRTFsec        */ {0,                fFalse, racChngProp,    irrbRsec},
/* iszRTFsect       */ {0,                fFalse, racSpecCharAct, ipfnDoSectEnd},
/* iszRTFsectd      */ {0,                fFalse, racChngProp,    irrbRsectd},

/* iszRTFshad       */ {0,                fFalse, racChngProp,    irrbRshad},

/* iszRTFsl         */ {0,                fFalse, racChngProp,    irrbRsl},
/* iszRTFsnext      */ {0,                fFalse, racChngProp,    irrbRsnext},
/* iszRTFstrike     */ {0,                fFalse, racChngProp,    irrbRstrike},
/* iszRTFstylesheet */ {0,                fFalse, racChngDest,    ipfnRdsStylesheet},
/* iszRTFsubject    */ {rdsSubject,       fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFtab        */ {0,                fFalse, racSpecCharAct, ipfnTab},
/* iszRTFtb         */ {0,                fFalse, racChngProp,    irrbRtb},
/* iszRTFtc         */ {rdsTc,            fTrue,  racChngDest,    ipfnRdsXeTc},
/* iszRTFtcelld     */ {0,                fFalse, racChngProp,    irrbRtcelld}
/* iszRTFtcf        */ {0,                fFalse, racSpecCharAct, ipfnTrnsfrmTcf},
/* iszRTFtcl        */ {0,                fFalse, racSpecCharAct, ipfnTrnsfrmTcl},
/* iszRTFtemplate   */ {rdsTemplate,      fTrue,  racChngDest,    ipfnRdsNewStd},
		/* this is really a dummy entry */
/* iszRTFtime       */ {0,                fFalse, racChngProp,    irrbRtime},
/* iszRTFtitle      */ {rdsTitle,         fTrue,  racChngDest,    ipfnRdsNewStd},
/* iszRTFtitlepg    */ {0,                fFalse, racChngProp,    irrbRtitlepg},
/* iszRTFtldot      */ {tlcDot,           fTrue,  racChngProp,    irrbRtlc},
/* iszRTFtlhyph     */ {tlcHyphen,        fTrue,  racChngProp,    irrbRtlc},
/* iszRTFtlth       */ {tlcSingle,        fTrue,  racChngProp,    irrbRtlc},
/* iszRTFtlul       */ {tlcSingle,        fTrue,  racChngProp,    irrbRtlc},
/* iszRTFtqc        */ {jcCenter,         fTrue,  racChngProp,    irrbRtjc},
/* iszRTFtqdec      */ {jcDecimal,        fTrue,  racChngProp,    irrbRtjc},
/* iszRTFtqr        */ {jcRight,          fTrue,  racChngProp,    irrbRtjc},
/* iszRTFtrgaph     */ {0,                fFalse, racChngProp,    irrbRtrgaph},
/* iszRTFtrleft     */ {0,                fFalse, racChngProp,    irrbRtrleft},
/* iszRTFtrowd      */ {0,                fFalse, racChngProp,    irrbRtrowd},
/* iszRTFtrqc       */ {jcCenter,         fTrue,  racChngProp,    irrbRtcjc},
/* iszRTFtrql       */ {jcLeft,           fTrue,  racChngProp,    irrbRtcjc},
/* iszRTFtrqr       */ {jcRight,          fTrue,  racChngProp,    irrbRtcjc},
/* iszRTFtrrh       */ {0,                fFalse, racChngProp,    irrbRtrrh},
/* iszRTFtx         */ {0,                fFalse, racChngProp,    irrbRtx},
/* iszRTFtxe        */ {rdsTxe,           fTrue,  racChngDest,    ipfnRdsXeRT},
/* iszRTFul         */ {kulSingle,        fTrue,  racChngProp,    irrbRkul},
/* iszRTFuld        */ {kulDotted,        fTrue,  racChngProp,    irrbRkul},
/* iszRTFuldb       */ {kulDouble,        fTrue,  racChngProp,    irrbRkul},
/* iszRTFulnone     */ {kulNone,          fTrue,  racChngProp,    irrbRkul},
/* iszRTFulw        */ {kulWord,          fTrue,  racChngProp,    irrbRkul},
/* iszRTFup         */ {0,                fFalse, racChngProp,    irrbRup},
/* iszRTFv          */ {0,                fFalse, racChngProp,    irrbRv},
/* iszRTFvern       */ {0,                fFalse, racChngProp,    irrbRvern},
/* iszRTFversion    */ {0,                fFalse, racChngProp,    irrbRversion},
/* iszRTFvertal     */ {vjcBottom,        fTrue,  racChngProp,    irrbRvjc},
/* iszRTFvertalc    */ {vjcCenter,        fTrue,  racChngProp,    irrbRvjc},
/* iszRTFvertalj    */ {vjcBoth,          fTrue,  racChngProp,    irrbRvjc},
/* iszRTFvertalt    */ {vjcTop,           fTrue,  racChngProp,    irrbRvjc},
/* iszRTFwbitmap    */ {0,                fFalse, racChngProp,    irrbRwbitmap},
/* iszRTFwbmbitspixel  */ {0,     fFalse, racChngProp,    irrbRwbmbitspixel},
/* iszRTFwbmplanes     */ {0,     fFalse, racChngProp,    irrbRwbmplanes},
/* iszRTFwbmwidthbytes */ {0,     fFalse, racChngProp,    irrbRwbmwidthbytes},
/* iszRTFwidowctrl  */ {0,                fFalse, racChngProp,    irrbRwidowctrl},
/* iszRTFwmetafile  */ {0,                fFalse, racChngProp,    irrbRwmetafile},
/* iszRTFxe         */ {rdsXe,            fTrue,  racChngDest,    ipfnRdsXeTc},
/* iszRTFyr         */ {0,                fFalse, racChngProp,    irrbRyr},
/* iszRTFLBracket   */ {chRTFOpenBrk,     fTrue,  racSpecChar,    0},
		/* note not same as mac - we only partially convert Mac formulas */
/* iszRTFFormula    */ {chBackslash,      fTrue,  racSpecChar,    0},
/* iszRTFRBracket   */ {chRTFCloseBrk,    fTrue,  racSpecChar,    0},
/* iszRTFTilde      */ {chNonBreakSpace,  fTrue,  racSpecChar,    0}
	};
#endif /* RTFIN */

/* *********************************************************** */
/* Next sets of tables up to irrb defines pass through rtfgen unchanged */
/* *********************************************************** */

#ifdef RTFOUT
/* multi- string fanout tables. Id's start after last valid id */

#define impjcisz       iszRTFMax + 0
#define impbrclisz     iszRTFMax + 1
#define impkulisz      iszRTFMax + 2
#define impfpcisz      iszRTFMax + 3
#define impbkcisz      iszRTFMax + 4
#define impnfcisz      iszRTFMax + 5
#define implncisz      iszRTFMax + 6
#define impvjcisz      iszRTFMax + 7
#define imppcVertisz   iszRTFMax + 8
#define imppcHorzisz   iszRTFMax + 9
#define impidxaAbsisz  iszRTFMax + 10
#define impidyaAbsisz  iszRTFMax + 11
#define imppgcisz      iszRTFMax + 12
#define imptcjcisz     iszRTFMax + 13
#define imptlcisz      iszRTFMax + 14
#define imptjcisz      iszRTFMax + 15
#define impihdtisz     iszRTFMax + 16

#define mpjcisz       (rgmpvalisz[impjcisz - iszRTFMax])
#define mpbrclisz     (rgmpvalisz[impbrclisz - iszRTFMax])
#define mpkulisz      (rgmpvalisz[impkulisz - iszRTFMax])
#define mpfpcisz      (rgmpvalisz[impfpcisz - iszRTFMax])
#define mpbkcisz      (rgmpvalisz[impbkcisz - iszRTFMax])
#define mpnfcisz      (rgmpvalisz[impnfcisz - iszRTFMax])
#define mplncisz      (rgmpvalisz[implncisz - iszRTFMax])
#define mpvjcisz      (rgmpvalisz[impvjcisz - iszRTFMax])
#define mppcVertisz   (rgmpvalisz[imppcVertisz - iszRTFMax])
#define mppcHorzisz   (rgmpvalisz[imppcHorzisz - iszRTFMax]) 
#define mpidxaAbsisz  (rgmpvalisz[impidxaAbsisz - iszRTFMax])
#define mpidyaAbsisz  (rgmpvalisz[impidyaAbsisz - iszRTFMax])
#define mppgcisz      (rgmpvalisz[imppgcisz - iszRTFMax])
#define mptcjcisz     (rgmpvalisz[imptcjcisz - iszRTFMax])
#define mptlcisz      (rgmpvalisz[imptlcisz - iszRTFMax])
#define mptjcisz      (rgmpvalisz[imptjcisz - iszRTFMax])
#define mpihdtisz     (rgmpvalisz[impihdtisz - iszRTFMax])

/* tables to handle fan out of a value into multiple rtf strings */
/* used by rtfout when generating a different string based on
	different values of a field; e.g. underline
*/

csconst int rgmpvalisz[][] =
	{
	/* mpjcisz[] */      {iszRTFql, iszRTFqc, iszRTFqr, iszRTFqj},
	/* mpbrclisz[] */    {iszRTFbrdrs, iszRTFbrdrth, iszRTFbrdrdb, iszRTFbrdrsh},
	/* mpkulisz[] */     {iszRTFul, iszRTFulw, iszRTFuldb, iszRTFuld},
	/* mpfpcisz[] */     {iszRTFendnotes, iszRTFftnbj, iszRTFftntj, iszRTFenddoc},
	/* mpbkcisz[] */     {iszRTFsbknone, iszRTFsbkcol, iszRTFsbkpage, iszRTFsbkeven,
							iszRTFsbkodd},
	/* mpnfcisz[] */     {iszRTFpgndec, iszRTFpgnucrm, iszRTFpgnlcrm, iszRTFpgnucltr,
							iszRTFpgnlcltr},
	/* mplncisz[] */     {iszRTFlineppage, iszRTFlinerestart, iszRTFlinecont},
	/* mpvjcisz[] */     {iszRTFvertalt, iszRTFvertalc, iszRTFvertalj, iszRTFvertal},
	/* mppcVertisz[] */  {iszRTFpvmrg, iszRTFpvpg},
	/* mppcHorzisz[] */  {iszRTFphcol, iszRTFphmrg, iszRTFphpg},
	/* mpidxaAbsisz[] */ {iszRTFposxl, iszRTFposxc, iszRTFposxr, iszRTFposxi,
							iszRTFposxo},
	/* mpidyaAbsisz[] */ {iszRTFposyil, iszRTFposyt, iszRTFposyc, iszRTFposyb},
	/* mppgcisz[] */     {iszRTFpgncont, iszRTFpgnrestart},
	/* mptcjcisz[] */    {iszRTFtrql, iszRTFtrqc, iszRTFtrqr},
	/* mptlcisz[] */     {iszRTFtldot, iszRTFtlhyph, iszRTFtlul, iszRTFtlul},
	/* mptjcisz[] */     {iszRTFtqc, iszRTFtqr, iszRTFtqdec, iszRTFtb},
	/* mpihdtisz[] */    {iszRTFheaderl, iszRTFheaderr, iszRTFfooterl,
							iszRTFfooterr, iszRTFheaderf, iszRTFfooterf, 
							iszRTFftnsep, iszRTFftnsepc, iszRTFftncn}
	};

csconst int rgvalMax[] = {jcBoth + 1, brclShadow + 1, kulDotted,
			fpcEndDoc + 1, bkcOddPage + 1, nfcLCLetter + 1,
			lncContinue + 1, vjcBottom + 1, pcVPage + 1, pcHPage + 1,
			5, 4, 2, jcRight + 1};

csconst int mpibrciszPap[] = {iszRTFbrdrt, iszRTFbrdrl, iszRTFbrdrb, iszRTFbrdrr,
		iszRTFbrdrbtw, iszRTFbrdrbar};
csconst int mpibrciszTc[] = {iszRTFclbrdrt, iszRTFclbrdrl, iszRTFclbrdrb, iszRTFclbrdrr};

#endif /* RTFOUT */


#ifdef RTFSUBS
csconst struct RRB rgrrbWord[] =

/* rrb RtfIn/Out tables - match keywords to structure locations */

/* WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING
*
* Watch out for rrbaBit entries. The bit offset is hard coded and may
* vary from machine to machine. The values are defined in rtf.n.
* There should be a test routine that checks that those values are
* correct for their structures, as the compiler can't do it.
*
*     Example: bit 15 on the Mac is bit 0 on the 8086!
*/



/*                        rrba:3,   pgc:7,   b:6,           w */
	{

/* irrbRs */            { rrbaSpec, pgcPara, pof(stc), 0 },
/* irrbRjc */           { rrbaByte, pgcPara, pof(jc), 0 },
/* irrbRfi */           { rrbaWord, pgcPara, pof(dxaLeft1), 0 },
/* irrbRli */           { rrbaWord, pgcPara, pof(dxaLeft), 0 },
/* irrbRri */           { rrbaWord, pgcPara, pof(dxaRight), 0 },
/* irrbRsb */           { rrbaUns,  pgcPara, pof(dyaBefore), 0 },
/* irrbRsa */           { rrbaUns,  pgcPara, pof(dyaAfter), 0 },
/* irrbRsl */           { rrbaWord, pgcPara, pof(dyaLine), 0 },
/* irrbRkeep */         { rrbaFlag, pgcPara, pof(fKeep), 0 },
/* irrbRkeepn */        { rrbaFlag, pgcPara, pof(fKeepFollow), 0 },
/* irrbRpagebb */       { rrbaFlag, pgcPara, pof(fPageBreakBefore), 0 },
/* irrbRnoline */       { rrbaFlag, pgcPara, pof(fNoLnn), 0 },
/* irrbRintbl */        { rrbaFlag, pgcPara, pof(fInTable), 0},
/* irrbRpbrc */         { rrbaSpec, pgcPara, 0, 0 },
/* irrbRpcVert */       { rrbaSpec, pgcPara, 0, 0 },
/* irrbRpcHorz */       { rrbaSpec, pgcPara, 0, 0 },
/* irrbRdxaAbs */       { rrbaSpec, pgcPara, pof(dxaAbs), 0},
/* irrbRdyaAbs */       { rrbaSpec, pgcPara, pof(dyaAbs), 0},
/* irrbRabsw */         { rrbaWord, pgcPara, pof(dxaWidth), 0},
/* irrbRdxfrtext */     { rrbaWord, pgcPara, pof(dxaFromText), 0},
/* irrbRtx */           { rrbaSpec, pgcPara, 0, 0 },
/* irrbRtlc */          { rrbaSpec, pgcPara, 0, 0 },
/* irrbRtjc */          { rrbaSpec, pgcPara, 0, 0 },
/* irrbRtb */           { rrbaSpec, pgcPara, 0, 0 },
/* irrbRdxaAbsNeg */    { rrbaSpec, pgcPara, 0, 0},
/* irrbRdyaAbsNeg */    { rrbaSpec, pgcPara, 0, 0},
/* irrbRpard */         { rrbaSpec, pgcPara, 0, 0 },
/* irrbRb */            { rrbaBit,  pgcChar, ibitCfBold, iwordCfBold },
/* irrbRi */            { rrbaBit,  pgcChar, ibitCfItalic, iwordCfItalic },
/* irrbRstrike */       { rrbaBit,  pgcChar, ibitCfStrike, iwordCfStrike },
/* irrbRoutl */         { rrbaBit,  pgcChar, ibitCfOutline, iwordCfOutline },
/* irrbRscaps */        { rrbaBit,  pgcChar, ibitCfSmallCaps, iwordCfSmallCaps },
/* irrbRv */            { rrbaSpec, pgcChar, ibitCfVanish, iwordCfVanish },
/* irrbRf */            { rrbaSpec, pgcChar, cof(ftc), ftcDefault /* Default */},
		/* we use RTF default 24, not Opus hpsDefault (20) */
/* irrbRfs */           { rrbaSpec, pgcChar, cof(hps), 24 },
/* irrbRkul */          { rrbaSpec, pgcChar, 0, 0 },
/* irrbRexpnd */        { rrbaSpec, pgcChar, 0, 0 },
/* irrbRcf */           { rrbaSpec, pgcChar, 0, 0}, 
/* irrbRup */           { rrbaSpec, pgcChar, cof(hpsPos), 6 },
/* irrbRrevised */      { rrbaBit,  pgcChar, ibitCfRMark, iwordCfRMark },
/* irrbRdn */           { rrbaSpec, pgcChar, cof(hpsPos), 6 },
/* irrbRplain */        { rrbaSpec, pgcChar, 0, 0 },
/* irrbRpaperw */       { rrbaWord, pgcDoc,  dof(xaPage), 12240 },
/* irrbRpaperh */       { rrbaWord, pgcDoc,  dof(yaPage), 15840 },
/* irrbRmargl */        { rrbaWord, pgcDoc,  dof(dxaLeft), 1800 },
/* irrbRmargr */        { rrbaWord, pgcDoc,  dof(dxaRight), 1800 },
/* irrbRmargt */        { rrbaWord, pgcDoc,  dof(dyaTop), 1440 },
/* irrbRmargb */        { rrbaWord, pgcDoc,  dof(dyaBottom), 1440 },
/* irrbRfacingp */      { rrbaBit,  pgcDoc,  ibitDfFacingPages, iwordDfFacingPages },
/* irrbRgutter */       { rrbaWord, pgcDoc,  dof(dxaGutter), 0 },
/* irrbRdeftab */       { rrbaWord, pgcDoc,  dof(dxaTab), 720 },
/* irrbRwidowctrl */    { rrbaBit,  pgcDoc,  ibitDfWidowControl, iwordDfWidowControl },
/* irrbRfpc */          { rrbaSpec, pgcDoc,  0, fpcEndnote },
/* irrbRftnstart */     { rrbaSpec, pgcDoc,  0, 1 },
/* irrbRftnrestart */   { rrbaBit,  pgcDoc,  ibitDfFtnRestart, iwordDfFtnRestart },
		/* pgn/line Start used to set the first section start page in Opus */
		/* stashed in ribl until end of input */
/* irrbRpgnstart */     { rrbaSpec, pgcRibl, riblof(pgnStart), 1 },
/* irrbRlinestart */    { rrbaSpec, pgcRibl, riblof(lineStart), 1 },
/* irrbRlandscape */    { rrbaBit,  pgcDoc,  ibitDfWide, iwordDfWide },
/* irrbRhyphhotz */     { rrbaWord, pgcDoc,  dof(dxaHotZ), 360 },
/* irrbRrevisions */    { rrbaBit,  pgcRibl, ibitRfRevisions, iwordRfRevisions },
/* irrbRrevbar */       { rrbaByte, pgcDoc,  dof(irmBar), irmBarDef },
/* irrbRrevprop */      { rrbaSpec, pgcDoc,  0, irmPropsDef },
/* irrbRmakebackup */   { rrbaBit,  pgcDoc,  ibitDfBackup, iwordDfBackup },
/* irrbRmargmirror */   { rrbaBit,  pgcDoc,  ibitDfMirrorMargins, iwordDfMirrorMargins},
/* irrbRbkc */          { rrbaByte, pgcSect, sof(bkc), bkcNewPage},
/* irrbRpgc */          { rrbaByte, pgcSect, sof(fPgnRestart), 0 },
/* irrbRpgnstarts */    { rrbaSpec, pgcSect, sof(pgnStart), 1 },
/* irrbRnfc */          { rrbaByte, pgcSect, sof(nfcPgn), nfcArabic },
/* irrbRlinemod */      { rrbaUns,  pgcSect, sof(nLnnMod), 0 },
/* irrbRlinex */        { rrbaUns,  pgcSect, sof(dxaLnn), 360 },
/* irrbRlnc */          { rrbaByte, pgcSect, sof(lnc), 0 },
/* irrbRlinestarts */   { rrbaSpec, pgcSect, sof(lnnMin), 1 },
/* irrbRheadery */      { rrbaUns,  pgcSect, sof(dyaHdrTop), 720 },
/* irrbRfootery */      { rrbaUns,  pgcSect, sof(dyaHdrBottom), 720 },
/* irrbRcols */         { rrbaSpec, pgcSect, sof(ccolM1), 1 },
/* irrbRcolsx */        { rrbaUns,  pgcSect, sof(dxaColumns), 720 },
/* irrbRlinebetcol */   { rrbaFlag, pgcSect, sof(fLBetween), 0 },
/* irrbRvjc */          { rrbaByte, pgcSect, sof(vjc), vjcTop },
/* irrbRendnhere */     { rrbaFlag, pgcSect, sof(fEndnote), 0 },
/* irrbRtitlepg */      { rrbaFlag, pgcSect, sof(fTitlePage), 0 },
/* irrbRsectd */        { rrbaSpec, pgcSect, 0, 0 },
/* irrbRsbys */         { rrbaSpec, pgcPara, pof(fSideBySide), 0 },

/* irrbRpgnx */		{ rrbaSpec, pgcRibl, 0, pgnxyDef },
/* irrbRpgny */		{ rrbaSpec, pgcRibl, 0, pgnxyDef },

/* irrbRpiccropb */     { rrbaWord, pgcPic,  picof(dyaCropBottom), 0 },
/* irrbRpiccropl */     { rrbaWord, pgcPic,  picof(dxaCropLeft), 0 },
/* irrbRpiccropr */     { rrbaWord, pgcPic,  picof(dxaCropRight), 0 },
/* irrbRpiccropt */     { rrbaWord, pgcPic,  picof(dyaCropTop), 0 },
/* irrbRpicscalex */    { rrbaWord, pgcPic,  picof(mx), 100 },
/* irrbRpicscaley */    { rrbaWord, pgcPic,  picof(my), 100 },
/* irrbRpich */         { rrbaSpec, pgcPic,  picof(bm.bmHeight), 0 },
/* irrbRpichGoal */     { rrbaWord, pgcPic,  picof(dyaGoal), 0 },
/* irrbRpicw */         { rrbaSpec, pgcPic,  picof(bm.bmWidth), 0 },
/* irrbRpicwGoal */     { rrbaWord, pgcPic,  picof(dxaGoal), 0 },
/* irrbRwbmbitspixel */ { rrbaByte, pgcPic,  picof(bm.bmBitsPixel), 1 },
/* irrbRwbmplanes    */ { rrbaByte, pgcPic,  picof(bm.bmPlanes), 1 },
/* irrbRwbmwidthbytes*/ { rrbaWord, pgcPic,  picof(bm.bmWidthBytes), 0 },
/* irrbRmacpict */      { rrbaSpec, pgcRibl, 0, 0 },
/* irrbRwbitmap   */    { rrbaSpec, pgcRibl, 0, 0 },
/* irrbRwmetafile */    { rrbaSpec, pgcRibl, 0, MM_TEXT },

/* irrbRchs */          { rrbaWord, pgcRibl, riblof(chs), 0 },
/* irrbRfmc */          { rrbaWord, pgcRibl, riblof(fmc), 0 },
/* irrbRblue */         { rrbaSpec, pgcRibl, riblof(cBlue), 0}, 
/* irrbRgreen */        { rrbaSpec, pgcRibl, riblof(cGreen), 0},
/* irrbRred */          { rrbaSpec, pgcRibl, riblof(cRed), 0},

/* irrbRsbasedon */     { rrbaByte, pgcRibl, riblof(estcp.stcBase), stcStdMin },
/* irrbRsnext */        { rrbaSpec, pgcRibl, riblof(estcp.stcNext), 0 },

/* irrbRflddirty */     { rrbaSpec, pgcRibl, ibitFFfResDirty, iwordFFfResDirty },
/* irrbRfldedit */      { rrbaSpec, pgcRibl, ibitFFfResEdit, iwordFFfResEdit },
/* irrbRfldlock */      { rrbaSpec, pgcRibl, ibitFFfLocked, iwordFFfLocked },
/* irrbRfldpriv */      { rrbaSpec, pgcRibl, ibitFFfPrivRes, iwordFFfPrivRes },
/* irrbRbrdrsh */       { rrbaSpec, pgcBrc,  0, 1},
/* irrbRbrsp */         { rrbaSpec, pgcBrc, 0, 0},
/* irrbRbrcBase */      { rrbaSpec, pgcBrc, 0, 0},
/* irrbRtcjc */         { rrbaWord, pgcTable, tapof(jc), 0},
/* irrbRtrgaph */       { rrbaWord, pgcTable, tapof(dxaGapHalf), 0},
/* irrbRtrrh */         { rrbaWord, pgcTable, tapof(dyaRowHeight), 0},
/* irrbRtrleft */       { rrbaWord, pgcTable, tapof(rgdxaCenter), 0},
/* irrbRclmgf */        { rrbaSpec, pgcTable, ibitTfFirstMerged, 1},
/* irrbRclmrg */        { rrbaSpec, pgcTable, ibitTfMerged, 1},

/* irrbRclbrc */        { rrbaSpec, pgcTable, 0, 0},
/* irrbRcellx */        { rrbaSpec, pgcTable, 0, 0},
/* irrbRtrowd */        { rrbaSpec, pgcTable, 0, 0},
/* irrbRtcelld */       { rrbaSpec, pgcTable, 0, 0},

/* irrbRdy  */          { rrbaWord, pgcDtr,  dtrof(dom), 0 },
/* irrbRhr  */          { rrbaWord, pgcDtr,  dtrof(hr), 0 },
/* irrbRmin */          { rrbaWord, pgcDtr,  dtrof(mint), 0 },
/* irrbRmo  */          { rrbaWord, pgcDtr,  dtrof(mon), 0 },
/* irrbRsec */          { rrbaWord, pgcDtr,  dtrof(sec), 0 },
/* irrbRyr  */          { rrbaWord, pgcDtr,  dtrof(yr), 0 },

/* irrbRversion */      { rrbaSpec, pgcDoc, 0, rdsVersion },
/* irrbRnofchars */     { rrbaSpec, pgcDoc, 0, rdsNofchars },
/* irrbRnofpages */     { rrbaSpec, pgcDoc, 0, rdsNofpages },
/* irrbRnofwords */     { rrbaSpec, pgcDoc, 0, rdsNofwords },
/* irrbRedmins */       { rrbaSpec, pgcDoc, 0, rdsEdmins },

/* irrbRg */            { rrbaWord, pgcRibl, riblof(iGrid), 0 },
/* irrbRgcw */          { rrbaSpec, pgcRibl, 0, 0 },

/* irrbRAsterisk */     { rrbaByte, pgcRibl, riblof(fNextIsDest), 1 },
/* irrbRdeff */         { rrbaSpec, pgcRibl, riblof(deff), 0 },
/* irrbRvern */			{ rrbaWord, pgcRibl, riblof(vern), 0 },
/* irrbRIgnore */       { rrbaSpec, 0,       0, 0 }

		/* unused in Opus */
/* irrbRpicscaled */
/* irrbRshad */
/* irrbRdefformat  */
/* irrbRfracwidth  */
/* irrbRpsover     */

/* irrbRnextfile */
/* irrbRid       */

/* irrbRdate     */
/* irrbRtime     */

	};

#endif /* RTFSUBS */



#ifdef RTFOUT
csconst int mpirrbiszWord[] =

/*  RtfOut table matching properties indicated by irrb to output string */

	{
	/* irrbRs */ iszRTFs,
	/* irrbRjc */ impjcisz,
	/* irrbRfi */ iszRTFfi,
	/* irrbRli */ iszRTFli,
	/* irrbRri */ iszRTFri,
	/* irrbRsb */ iszRTFsb,
	/* irrbRsa */ iszRTFsa,
	/* irrbRsl */ iszRTFsl,
	/* irrbRkeep */ iszRTFkeep,
	/* irrbRkeepn */ iszRTFkeepn,
	/* irrbRpagebb */ iszRTFpagebb,
	/* irrbRnoline */ iszRTFnoline,
	/* irrbRintbl */ iszRTFintbl,
	/* irrbRpbrc  */ 0,
	/* irrbRpcVert*/ imppcVertisz,
	/* irrbRpcHorz */ imppcHorzisz,
	/* irrbRdxaAbs */ impidxaAbsisz,
	/* irrbRdyaAbs */ impidyaAbsisz,
	/* irrbRabsw */ iszRTFabsw,
	/* irrbRdxfrtext */ iszRTFdxfrtext,
	/* irrbRtx */ iszRTFtx,
	/* irrbRtlc */ 0,
	/* irrbRtjc */ 0,
	/* irrbRtb  */ 0,
	/* irrbRdxaAbsNeg */ 0,
	/* irrbRdyaAbsNeg */ 0,
	/* irrbRpard */ 0,
	/* irrbRb */ iszRTFb,
	/* irrbRi */ iszRTFi,
	/* irrbRstrike */ iszRTFstrike,
	/* irrbRoutl */ iszRTFoutl,
	/* irrbRscaps */ iszRTFscaps,
	/* irrbRv */ iszRTFv,
	/* irrbRf */ iszRTFf,
	/* irrbRfs */ iszRTFfs,
	/* irrbRkul */ impkulisz,
	/* irrbRexpnd */ iszRTFexpnd,
	/* irrbRcf */ iszRTFcf,
	/* irrbRup */ iszRTFup,
	/* irrbRrevised */ iszRTFrevised,
	/* irrbRdn */ iszRTFdn,
	/* irrbRplain */ 0,
	/* irrbRpaperw */ iszRTFpaperw,
	/* irrbRpaperh */ iszRTFpaperh,
	/* irrbRmargl */ iszRTFmargl,
	/* irrbRmargr */ iszRTFmargr,
	/* irrbRmargt */ iszRTFmargt,
	/* irrbRmargb */ iszRTFmargb,
	/* irrbRfacingp */ iszRTFfacingp,
	/* irrbRgutter */ iszRTFgutter,
	/* irrbRdeftab */ iszRTFdeftab,
	/* irrbRwidowctrl */ iszRTFwidowctrl,
	/* irrbRfpc */ impfpcisz,
	/* irrbRftnstart */ iszRTFftnstart,
	/* irrbRftnrestart */ iszRTFftnrestart,
	/* irrbRpgnstart */ iszRTFpgnstart,
	/* irrbRlinestart */ iszRTFlinestart,
	/* irrbRlandscape */ iszRTFlandscape,
	/* irrbRhyphhotz */  iszRTFhyphhotz,
	/* irrbRrevisions */ iszRTFrevisions,
	/* irrbRrevbar */    iszRTFrevbar,
	/* irrbRrevprop */   iszRTFrevprop,
	/* irrbRmakebackup */ iszRTFmakebackup,
	/* irrbRmargmirror */ iszRTFmargmirror,
	/* irrbRbkc */       impbkcisz,
	/* irrbRpgc */ imppgcisz,
	/* irrbRpgnstarts */ iszRTFpgnstarts,
	/* irrbRnfc */ impnfcisz,
	/* irrbRlinemod */ iszRTFlinemod,
	/* irrbRlinex */ iszRTFlinex,
	/* irrbRlnc */ implncisz,
	/* irrbRlinestarts */ iszRTFlinestarts,
	/* irrbRheadery */ iszRTFheadery,
	/* irrbRfootery */ iszRTFfootery,
	/* irrbRcols */ iszRTFcols,
	/* irrbRcolsx */ iszRTFcolsx,
	/* irrbRlinebetcol */ iszRTFlinebetcol,
	/* irrbRvjc */ impvjcisz,
	/* irrbRendnhere */ iszRTFendnhere,
	/* irrbRtitlepg */ iszRTFtitlepg,
	/* irrbRsectd */        0,
	/* irrbRsbys */ iszRTFsbys,
	/* irrbRpgnx */ 0,
	/* irrbRpgny */ 0,

	/* irrbRpiccropb */     0,
	/* irrbRpiccropl */     0,
	/* irrbRpiccropr */     0,
	/* irrbRpiccropt */     0,
	/* irrbRpicscalex */    0,
	/* irrbRpicscaley */    0,
	/* irrbRpich */         0,
	/* irrbRpichGoal */     0,
	/* irrbRpicw */         0,
	/* irrbRpicwGoal */     0,
	/* irrbRwbmbitspixel */   0,
	/* irrbRwbmplanes */    0,
	/* irrbRwbmwidthbytes */  0,
	/* irrbRmacpict */      0,
	/* irrbRwbitmap  */     0,
	/* irrbRwmetafile */    0,

	/* irrbRchs */          0,
	/* irrbRfmc */          0,
	/* irrbRblue */         0,
	/* irrbRgreen */        0,
	/* irrbRred */          0,
	/* irrbRsbasedon */     0,
	/* irrbRsnext */        0,
	/* irrbRflddirty */     0,
	/* irrbRfldedit */      0,
	/* irrbRfldlock */      0,
	/* irrbRfldpriv */      0,
	/* irrbRbrdrsh */       iszRTFbrdrsh,
	/* irrbRbrsp */         iszRTFbrsp,
	/* irrbRbrcBase */      0,
	/* irrbRtcjc */      imptcjcisz,
	/* irrbRtrgaph */    iszRTFtrgaph,
	/* irrbRtrrh */      iszRTFtrrh,
	/* irrbRtrleft */    iszRTFtrleft,
	/* irrbRclmgf */     iszRTFclmgf,
	/* irrbRclmrg */     iszRTFclmrg
	/* irrbRclbrc */
	/* irrbRcellx */
	/* irrbRtrowd */
	/* irrbRtcelld */
	/* irrbRdy  */
	/* irrbRhr  */
	/* irrbRmin */
	/* irrbRmo  */
	/* irrbRsec */
	/* irrbRyr  */
	/* irrbRversion */
	/* irrbRnofchars */
	/* irrbRnofpages */
	/* irrbRnofwords */
	/* irrbRedmins */
	/* irrbRg */
	/* irrbRgcw */
	/* irrbRAsterisk */
	/* irrbRdeff */
	/* irrbRvern */

	/* irrbRIgnore */

	/* irrbRpicscaled */
	/* irrbRshad */
	/* irrbRdefformat  */
	/* irrbRfracwidth  */
	/* irrbRpsover     */

	/* irrbRnextfile */
	/* irrbRid       */
	/* irrbRdate     */
	/* irrbRtime     */

	/* note: table ends at titlepg because the other strings corresponding
		to the rest of the rgrrbWord are not generated by table driven code.
		In general, this table is used in PropsToRtf to dump all the
		properties of a type (doc, para, sect, char) at once.
	*/

	};

#endif /* RTFOUT */

#endif /* RTFTABLES  */
