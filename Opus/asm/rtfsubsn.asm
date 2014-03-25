        include w2.inc
        include noxport.inc
        include consts.inc
        include structs.inc

createSeg	rtfsubs_PCODE,rtfsubsn,byte,public,CODE

; DEBUGGING DECLARATIONS

ifdef DEBUG
midRtfsubsn	equ 29		 ; module ID, for native asserts
endif

; EXTERNAL FUNCTIONS

externFP	<IInPlc>
externFP	<IInPlcRef>
externFP	<CpPlc>
externFP	<GetPlc>
externFP	<N_QcpQfooPlcfoo>
ifdef DEBUG
externFP	<AssertProcForNative>
endif ;DEBUG


sBegin  data

; EXTERNALS

externW     mpsbps

sEnd    data

; CODE SEGMENT _EDIT

sBegin	rtfsubsn
	assumes cs,rtfsubsn
        assumes ds,dgroup
        assumes ss,dgroup

szRTFlinefeed:
	db	00Ah,0
szRTFcarriagereturn:
	db	00Dh,0
szRTFquote:
	db	'''',0
szRTFasterisk:
	db	'*',0
szRTFhyphen:
	db	'-',0
szRTFcolon:
	db	':',0
szRTFbackslash:
	db	'\',0
szRTFunderscore:
	db	'_',0
szRTFabsw:
	db	'absw',0
szRTFannotation:
	db	'annotation',0
szRTFansi:
	db	'ansi',0
szRTFatnid:
	db	'atnid',0
szRTFauthor:
	db	'author',0
szRTFb:
	db	'b',0
szRTFbin:
	db	'bin',0
szRTFbkmkend:
	db	'bkmkend',0
szRTFbkmkstart:
	db	'bkmkstart',0
szRTFblue:
	db	'blue',0
szRTFbox:
	db	'box',0
szRTFbrdrb:
	db	'brdrb',0
szRTFbrdrbar:
	db	'brdrbar',0
szRTFbrdrbtw:
	db	'brdrbtw',0
szRTFbrdrdb:
	db	'brdrdb',0
szRTFbrdrdot:
	db	'brdrdot',0
szRTFbrdrhair:
	db	'brdrhair',0
szRTFbrdrl:
	db	'brdrl',0
szRTFbrdrr:
	db	'brdrr',0
szRTFbrdrs:
	db	'brdrs',0
szRTFbrdrsh:
	db	'brdrsh',0
szRTFbrdrt:
	db	'brdrt',0
szRTFbrdrth:
	db	'brdrth',0
szRTFbrsp:
	db	'brsp',0
szRTFbuptim:
	db	'buptim',0
szRTFbxe:
	db	'bxe',0
szRTFcell:
	db	'cell',0
szRTFcellx:
	db	'cellx',0
szRTFcf:
	db	'cf',0
szRTFchatn:
	db	'chatn',0
szRTFchdate:
	db	'chdate',0
szRTFchftn:
	db	'chftn',0
szRTFchftnsep:
	db	'chftnsep',0
szRTFchftnsepc:
	db	'chftnsepc',0
szRTFchpgn:
	db	'chpgn',0
szRTFchtime:
	db	'chtime',0
szRTFclbrdrb:
	db	'clbrdrb',0
szRTFclbrdrl:
	db	'clbrdrl',0
szRTFclbrdrr:
	db	'clbrdrr',0
szRTFclbrdrt:
	db	'clbrdrt',0
szRTFclmgf:
	db	'clmgf',0
szRTFclmrg:
	db	'clmrg',0
szRTFcolortbl:
	db	'colortbl',0
szRTFcols:
	db	'cols',0
szRTFcolsx:
	db	'colsx',0
szRTFcolumn:
	db	'column',0
szRTFcomment:
	db	'comment',0
szRTFcreatim:
	db	'creatim',0
szRTFdate:
	db	'date',0
szRTFdeff:
	db	'deff',0
szRTFdefformat:
	db	'defformat',0
szRTFdeftab:
	db	'deftab',0
szRTFdn:
	db	'dn',0
szRTFdoccomm:
	db	'doccomm',0
szRTFdxfrtext:
	db	'dxfrtext',0
szRTFdy:
	db	'dy',0
szRTFedmins:
	db	'edmins',0
szRTFenddoc:
	db	'enddoc',0
szRTFendnhere:
	db	'endnhere',0
szRTFendnotes:
	db	'endnotes',0
szRTFexpnd:
	db	'expnd',0
szRTFf:
	db	'f',0
szRTFfacingp:
	db	'facingp',0
szRTFfdecor:
	db	'fdecor',0
szRTFfi:
	db	'fi',0
szRTFfield:
	db	'field',0
szRTFflddirty:
	db	'flddirty',0
szRTFfldedit:
	db	'fldedit',0
szRTFfldinst:
	db	'fldinst',0
szRTFfldlock:
	db	'fldlock',0
szRTFfldpriv:
	db	'fldpriv',0
szRTFfldrslt:
	db	'fldrslt',0
szRTFfldtype:
	db	'fldtype',0
szRTFfmodern:
	db	'fmodern',0
szRTFfnil:
	db	'fnil',0
szRTFfonttbl:
	db	'fonttbl',0
szRTFfooter:
	db	'footer',0
szRTFfooterf:
	db	'footerf',0
szRTFfooterl:
	db	'footerl',0
szRTFfooterr:
	db	'footerr',0
szRTFfootery:
	db	'footery',0
szRTFfootnote:
	db	'footnote',0
szRTFfracwidth:
	db	'fracwidth',0
szRTFfroman:
	db	'froman',0
szRTFfs:
	db	'fs',0
szRTFfscript:
	db	'fscript',0
szRTFfswiss:
	db	'fswiss',0
szRTFftech:
	db	'ftech',0
szRTFftnbj:
	db	'ftnbj',0
szRTFftncn:
	db	'ftncn',0
szRTFftnrestart:
	db	'ftnrestart',0
szRTFftnsep:
	db	'ftnsep',0
szRTFftnsepc:
	db	'ftnsepc',0
szRTFftnstart:
	db	'ftnstart',0
szRTFftntj:
	db	'ftntj',0
szRTFg:
	db	'g',0
szRTFgcw:
	db	'gcw',0
szRTFgreen:
	db	'green',0
szRTFgridtbl:
	db	'gridtbl',0
szRTFgutter:
	db	'gutter',0
szRTFheader:
	db	'header',0
szRTFheaderf:
	db	'headerf',0
szRTFheaderl:
	db	'headerl',0
szRTFheaderr:
	db	'headerr',0
szRTFheadery:
	db	'headery',0
szRTFhr:
	db	'hr',0
szRTFhyphhotz:
	db	'hyphhotz',0
szRTFi:
	db	'i',0
szRTFid:
	db	'id',0
szRTFinfo:
	db	'info',0
szRTFintbl:
	db	'intbl',0
szRTFixe:
	db	'ixe',0
szRTFkeep:
	db	'keep',0
szRTFkeepn:
	db	'keepn',0
szRTFkeywords:
	db	'keywords',0
szRTFlandscape:
	db	'landscape',0
szRTFli:
	db	'li',0
szRTFline:
	db	'line',0
szRTFlinebetcol:
	db	'linebetcol',0
szRTFlinecont:
	db	'linecont',0
szRTFlinemod:
	db	'linemod',0
szRTFlineppage:
	db	'lineppage',0
szRTFlinerestart:
	db	'linerestart',0
szRTFlinestart:
	db	'linestart',0
szRTFlinestarts:
	db	'linestarts',0
szRTFlinex:
	db	'linex',0
szRTFmac:
	db	'mac',0
szRTFmacpict:
	db	'macpict',0
szRTFmakebackup:
	db	'makebackup',0
szRTFmargb:
	db	'margb',0
szRTFmargl:
	db	'margl',0
szRTFmargmirror:
	db	'margmirror',0
szRTFmargr:
	db	'margr',0
szRTFmargt:
	db	'margt',0
szRTFmin:
	db	'min',0
szRTFmo:
	db	'mo',0
szRTFnextfile:
	db	'nextfile',0
szRTFnofchars:
	db	'nofchars',0
szRTFnofpages:
	db	'nofpages',0
szRTFnofwords:
	db	'nofwords',0
szRTFnoline:
	db	'noline',0
szRTFoperator:
	db	'operator',0
szRTFoutl:
	db	'outl',0
szRTFpage:
	db	'page',0
szRTFpagebb:
	db	'pagebb',0
szRTFpaperh:
	db	'paperh',0
szRTFpaperw:
	db	'paperw',0
szRTFpar:
	db	'par',0
szRTFpard:
	db	'pard',0
szRTFpc:
	db	'pc',0
szRTFpca:
	db	'pca',0
szRTFpgncont:
	db	'pgncont',0
szRTFpgndec:
	db	'pgndec',0
szRTFpgnlcltr:
	db	'pgnlcltr',0
szRTFpgnlcrm:
	db	'pgnlcrm',0
szRTFpgnrestart:
	db	'pgnrestart',0
szRTFpgnstart:
	db	'pgnstart',0
szRTFpgnstarts:
	db	'pgnstarts',0
szRTFpgnucltr:
	db	'pgnucltr',0
szRTFpgnucrm:
	db	'pgnucrm',0
szRTFpgnx:
	db	'pgnx',0
szRTFpgny:
	db	'pgny',0
szRTFphcol:
	db	'phcol',0
szRTFphmrg:
	db	'phmrg',0
szRTFphpg:
	db	'phpg',0
szRTFpiccropb:
	db	'piccropb',0
szRTFpiccropl:
	db	'piccropl',0
szRTFpiccropr:
	db	'piccropr',0
szRTFpiccropt:
	db	'piccropt',0
szRTFpich:
	db	'pich',0
szRTFpichGoal:
	db	'pichGoal',0
szRTFpicscaled:
	db	'picscaled',0
szRTFpicscalex:
	db	'picscalex',0
szRTFpicscaley:
	db	'picscaley',0
szRTFpict:
	db	'pict',0
szRTFpicw:
	db	'picw',0
szRTFpicwGoal:
	db	'picwGoal',0
szRTFplain:
	db	'plain',0
szRTFposx:
	db	'posx',0
szRTFposxc:
	db	'posxc',0
szRTFposxi:
	db	'posxi',0
szRTFposxl:
	db	'posxl',0
szRTFposxo:
	db	'posxo',0
szRTFposxr:
	db	'posxr',0
szRTFposy:
	db	'posy',0
szRTFposyb:
	db	'posyb',0
szRTFposyc:
	db	'posyc',0
szRTFposyil:
	db	'posyil',0
szRTFposyt:
	db	'posyt',0
szRTFprintim:
	db	'printim',0
szRTFprivate:
	db	'private',0
szRTFpsover:
	db	'psover',0
szRTFpvmrg:
	db	'pvmrg',0
szRTFpvpg:
	db	'pvpg',0
szRTFqc:
	db	'qc',0
szRTFqj:
	db	'qj',0
szRTFql:
	db	'ql',0
szRTFqr:
	db	'qr',0
szRTFred:
	db	'red',0
szRTFrevbar:
	db	'revbar',0
szRTFrevised:
	db	'revised',0
szRTFrevisions:
	db	'revisions',0
szRTFrevprop:
	db	'revprop',0
szRTFrevtim:
	db	'revtim',0
szRTFri:
	db	'ri',0
szRTFrow:
	db	'row',0
szRTFrtf:
	db	'rtf',0
szRTFrxe:
	db	'rxe',0
szRTFs:
	db	's',0
szRTFsa:
	db	'sa',0
szRTFsb:
	db	'sb',0
szRTFsbasedon:
	db	'sbasedon',0
szRTFsbkcol:
	db	'sbkcol',0
szRTFsbkeven:
	db	'sbkeven',0
szRTFsbknone:
	db	'sbknone',0
szRTFsbkodd:
	db	'sbkodd',0
szRTFsbkpage:
	db	'sbkpage',0
szRTFsbys:
	db	'sbys',0
szRTFscaps:
	db	'scaps',0
szRTFsec:
	db	'sec',0
szRTFsect:
	db	'sect',0
szRTFsectd:
	db	'sectd',0
szRTFshad:
	db	'shad',0
szRTFsl:
	db	'sl',0
szRTFsnext:
	db	'snext',0
szRTFstrike:
	db	'strike',0
szRTFstylesheet:
	db	'stylesheet',0
szRTFsubject:
	db	'subject',0
szRTFtab:
	db	'tab',0
szRTFtb:
	db	'tb',0
szRTFtc:
	db	'tc',0
szRTFtcelld:
	db	'tcelld',0
szRTFtcf:
	db	'tcf',0
szRTFtcl:
	db	'tcl',0
szRTFtemplate:
	db	'template',0
szRTFtime:
	db	'time',0
szRTFtitle:
	db	'title',0
szRTFtitlepg:
	db	'titlepg',0
szRTFtldot:
	db	'tldot',0
szRTFtlhyph:
	db	'tlhyph',0
szRTFtlth:
	db	'tlth',0
szRTFtlul:
	db	'tlul',0
szRTFtqc:
	db	'tqc',0
szRTFtqdec:
	db	'tqdec',0
szRTFtqr:
	db	'tqr',0
szRTFtrgaph:
	db	'trgaph',0
szRTFtrleft:
	db	'trleft',0
szRTFtrowd:
	db	'trowd',0
szRTFtrqc:
	db	'trqc',0
szRTFtrql:
	db	'trql',0
szRTFtrqr:
	db	'trqr',0
szRTFtrrh:
	db	'trrh',0
szRTFtx:
	db	'tx',0
szRTFtxe:
	db	'txe',0
szRTFul:
	db	'ul',0
szRTFuld:
	db	'uld',0
szRTFuldb:
	db	'uldb',0
szRTFulnone:
	db	'ulnone',0
szRTFulw:
	db	'ulw',0
szRTFup:
	db	'up',0
szRTFv:
	db	'v',0
szRTFvern:
	db	'vern',0
szRTFversion:
	db	'version',0
szRTFvertal:
	db	'vertal',0
szRTFvertalc:
	db	'vertalc',0
szRTFvertalj:
	db	'vertalj',0
szRTFvertalt:
	db	'vertalt',0
szRTFwbitmap:
	db	'wbitmap',0
szRTFwbmbitspixel:
	db	'wbmbitspixel',0
szRTFwbmplanes:
	db	'wbmplanes',0
szRTFwbmwidthbytes:
	db	'wbmwidthbytes',0
szRTFwidowctrl:
	db	'widowctrl',0
szRTFwmetafile:
	db	'wmetafile',0
szRTFxe:
	db	'xe',0
szRTFyr:
	db	'yr',0
szRTFopenbrace:
	db	'{',0
szRTFverticalbar:
	db	'|',0
szRTFclosebrace:
	db	'}',0
szRTFtilde:
	db	'~',0

rgszRtfSym label word
	dw	offset szRTFlinefeed
	dw	offset szRTFcarriagereturn
	dw	offset szRTFquote
	dw	offset szRTFasterisk
	dw	offset szRTFhyphen
	dw	offset szRTFcolon
	dw	offset szRTFbackslash
	dw	offset szRTFunderscore
	dw	offset szRTFabsw
	dw	offset szRTFannotation
	dw	offset szRTFansi
	dw	offset szRTFatnid
	dw	offset szRTFauthor
	dw	offset szRTFb
	dw	offset szRTFbin
	dw	offset szRTFbkmkend
	dw	offset szRTFbkmkstart
	dw	offset szRTFblue
	dw	offset szRTFbox
	dw	offset szRTFbrdrb
	dw	offset szRTFbrdrbar
	dw	offset szRTFbrdrbtw
	dw	offset szRTFbrdrdb
	dw	offset szRTFbrdrdot
	dw	offset szRTFbrdrhair
	dw	offset szRTFbrdrl
	dw	offset szRTFbrdrr
	dw	offset szRTFbrdrs
	dw	offset szRTFbrdrsh
	dw	offset szRTFbrdrt
	dw	offset szRTFbrdrth
	dw	offset szRTFbrsp
	dw	offset szRTFbuptim
	dw	offset szRTFbxe
	dw	offset szRTFcell
	dw	offset szRTFcellx
	dw	offset szRTFcf
	dw	offset szRTFchatn
	dw	offset szRTFchdate
	dw	offset szRTFchftn
	dw	offset szRTFchftnsep
	dw	offset szRTFchftnsepc
	dw	offset szRTFchpgn
	dw	offset szRTFchtime
	dw	offset szRTFclbrdrb
	dw	offset szRTFclbrdrl
	dw	offset szRTFclbrdrr
	dw	offset szRTFclbrdrt
	dw	offset szRTFclmgf
	dw	offset szRTFclmrg
	dw	offset szRTFcolortbl
	dw	offset szRTFcols
	dw	offset szRTFcolsx
	dw	offset szRTFcolumn
	dw	offset szRTFcomment
	dw	offset szRTFcreatim
	dw	offset szRTFdate
	dw	offset szRTFdeff
	dw	offset szRTFdefformat
	dw	offset szRTFdeftab
	dw	offset szRTFdn
	dw	offset szRTFdoccomm
	dw	offset szRTFdxfrtext
	dw	offset szRTFdy
	dw	offset szRTFedmins
	dw	offset szRTFenddoc
	dw	offset szRTFendnhere
	dw	offset szRTFendnotes
	dw	offset szRTFexpnd
	dw	offset szRTFf
	dw	offset szRTFfacingp
	dw	offset szRTFfdecor
	dw	offset szRTFfi
	dw	offset szRTFfield
	dw	offset szRTFflddirty
	dw	offset szRTFfldedit
	dw	offset szRTFfldinst
	dw	offset szRTFfldlock
	dw	offset szRTFfldpriv
	dw	offset szRTFfldrslt
	dw	offset szRTFfldtype
	dw	offset szRTFfmodern
	dw	offset szRTFfnil
	dw	offset szRTFfonttbl
	dw	offset szRTFfooter
	dw	offset szRTFfooterf
	dw	offset szRTFfooterl
	dw	offset szRTFfooterr
	dw	offset szRTFfootery
	dw	offset szRTFfootnote
	dw	offset szRTFfracwidth
	dw	offset szRTFfroman
	dw	offset szRTFfs
	dw	offset szRTFfscript
	dw	offset szRTFfswiss
	dw	offset szRTFftech
	dw	offset szRTFftnbj
	dw	offset szRTFftncn
	dw	offset szRTFftnrestart
	dw	offset szRTFftnsep
	dw	offset szRTFftnsepc
	dw	offset szRTFftnstart
	dw	offset szRTFftntj
	dw	offset szRTFg
	dw	offset szRTFgcw
	dw	offset szRTFgreen
	dw	offset szRTFgridtbl
	dw	offset szRTFgutter
	dw	offset szRTFheader
	dw	offset szRTFheaderf
	dw	offset szRTFheaderl
	dw	offset szRTFheaderr
	dw	offset szRTFheadery
	dw	offset szRTFhr
	dw	offset szRTFhyphhotz
	dw	offset szRTFi
	dw	offset szRTFid
	dw	offset szRTFinfo
	dw	offset szRTFintbl
	dw	offset szRTFixe
	dw	offset szRTFkeep
	dw	offset szRTFkeepn
	dw	offset szRTFkeywords
	dw	offset szRTFlandscape
	dw	offset szRTFli
	dw	offset szRTFline
	dw	offset szRTFlinebetcol
	dw	offset szRTFlinecont
	dw	offset szRTFlinemod
	dw	offset szRTFlineppage
	dw	offset szRTFlinerestart
	dw	offset szRTFlinestart
	dw	offset szRTFlinestarts
	dw	offset szRTFlinex
	dw	offset szRTFmac
	dw	offset szRTFmacpict
	dw	offset szRTFmakebackup
	dw	offset szRTFmargb
	dw	offset szRTFmargl
	dw	offset szRTFmargmirror
	dw	offset szRTFmargr
	dw	offset szRTFmargt
	dw	offset szRTFmin
	dw	offset szRTFmo
	dw	offset szRTFnextfile
	dw	offset szRTFnofchars
	dw	offset szRTFnofpages
	dw	offset szRTFnofwords
	dw	offset szRTFnoline
	dw	offset szRTFoperator
	dw	offset szRTFoutl
	dw	offset szRTFpage
	dw	offset szRTFpagebb
	dw	offset szRTFpaperh
	dw	offset szRTFpaperw
	dw	offset szRTFpar
	dw	offset szRTFpard
	dw	offset szRTFpc
	dw	offset szRTFpca
	dw	offset szRTFpgncont
	dw	offset szRTFpgndec
	dw	offset szRTFpgnlcltr
	dw	offset szRTFpgnlcrm
	dw	offset szRTFpgnrestart
	dw	offset szRTFpgnstart
	dw	offset szRTFpgnstarts
	dw	offset szRTFpgnucltr
	dw	offset szRTFpgnucrm
	dw	offset szRTFpgnx
	dw	offset szRTFpgny
	dw	offset szRTFphcol
	dw	offset szRTFphmrg
	dw	offset szRTFphpg
	dw	offset szRTFpiccropb
	dw	offset szRTFpiccropl
	dw	offset szRTFpiccropr
	dw	offset szRTFpiccropt
	dw	offset szRTFpich
	dw	offset szRTFpichGoal
	dw	offset szRTFpicscaled
	dw	offset szRTFpicscalex
	dw	offset szRTFpicscaley
	dw	offset szRTFpict
	dw	offset szRTFpicw
	dw	offset szRTFpicwGoal
	dw	offset szRTFplain
	dw	offset szRTFposx
	dw	offset szRTFposxc
	dw	offset szRTFposxi
	dw	offset szRTFposxl
	dw	offset szRTFposxo
	dw	offset szRTFposxr
	dw	offset szRTFposy
	dw	offset szRTFposyb
	dw	offset szRTFposyc
	dw	offset szRTFposyil
	dw	offset szRTFposyt
	dw	offset szRTFprintim
	dw	offset szRTFprivate
	dw	offset szRTFpsover
	dw	offset szRTFpvmrg
	dw	offset szRTFpvpg
	dw	offset szRTFqc
	dw	offset szRTFqj
	dw	offset szRTFql
	dw	offset szRTFqr
	dw	offset szRTFred
	dw	offset szRTFrevbar
	dw	offset szRTFrevised
	dw	offset szRTFrevisions
	dw	offset szRTFrevprop
	dw	offset szRTFrevtim
	dw	offset szRTFri
	dw	offset szRTFrow
	dw	offset szRTFrtf
	dw	offset szRTFrxe
	dw	offset szRTFs
	dw	offset szRTFsa
	dw	offset szRTFsb
	dw	offset szRTFsbasedon
	dw	offset szRTFsbkcol
	dw	offset szRTFsbkeven
	dw	offset szRTFsbknone
	dw	offset szRTFsbkodd
	dw	offset szRTFsbkpage
	dw	offset szRTFsbys
	dw	offset szRTFscaps
	dw	offset szRTFsec
	dw	offset szRTFsect
	dw	offset szRTFsectd
	dw	offset szRTFshad
	dw	offset szRTFsl
	dw	offset szRTFsnext
	dw	offset szRTFstrike
	dw	offset szRTFstylesheet
	dw	offset szRTFsubject
	dw	offset szRTFtab
	dw	offset szRTFtb
	dw	offset szRTFtc
	dw	offset szRTFtcelld
	dw	offset szRTFtcf
	dw	offset szRTFtcl
	dw	offset szRTFtemplate
	dw	offset szRTFtime
	dw	offset szRTFtitle
	dw	offset szRTFtitlepg
	dw	offset szRTFtldot
	dw	offset szRTFtlhyph
	dw	offset szRTFtlth
	dw	offset szRTFtlul
	dw	offset szRTFtqc
	dw	offset szRTFtqdec
	dw	offset szRTFtqr
	dw	offset szRTFtrgaph
	dw	offset szRTFtrleft
	dw	offset szRTFtrowd
	dw	offset szRTFtrqc
	dw	offset szRTFtrql
	dw	offset szRTFtrqr
	dw	offset szRTFtrrh
	dw	offset szRTFtx
	dw	offset szRTFtxe
	dw	offset szRTFul
	dw	offset szRTFuld
	dw	offset szRTFuldb
	dw	offset szRTFulnone
	dw	offset szRTFulw
	dw	offset szRTFup
	dw	offset szRTFv
	dw	offset szRTFvern
	dw	offset szRTFversion
	dw	offset szRTFvertal
	dw	offset szRTFvertalc
	dw	offset szRTFvertalj
	dw	offset szRTFvertalt
	dw	offset szRTFwbitmap
	dw	offset szRTFwbmbitspixel
	dw	offset szRTFwbmplanes
	dw	offset szRTFwbmwidthbytes
	dw	offset szRTFwidowctrl
	dw	offset szRTFwmetafile
	dw	offset szRTFxe
	dw	offset szRTFyr
	dw	offset szRTFopenbrace
	dw	offset szRTFverticalbar
	dw	offset szRTFclosebrace
	dw	offset szRTFtilde
rgszRtfSymMax:
iszRTFMax = ((offset rgszRtfSymMax) - (offset rgszRtfSym)) SHR 1


;-------------------------------------------------------------------------
;	PchSzRtfMove(iszRtf, pch)
;-------------------------------------------------------------------------
;/* P C H   S Z   R T F   M O V E */
;HANDNATIVE char *C_PchSzRtfMove(iszRtf, pch)
;int iszRtf;
;char *pch;
;{
; %%Function:N_PchSzRtfMove %%Owner:BRADV
PUBLIC N_PchSzRtfMove
N_PchSzRtfMove:
	mov	bx,sp
	mov	cx,[bx+4]
	mov	bx,[bx+6]
ifdef PROFILE
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	call	StartNMeas
endif ;PROFILE
	push	si	;save caller's si
	push	di	;save caller's di

;	 char far *qch = rgszRtfSym[iszRtf];
;	 int ch;
	mov	si,bx
	mov	si,[bx.si.rgszRtfSym]
	mov	di,cx
	push	ds
	pop	es
	push	cs
	pop	ds
	db	0A8h	;turns next "stosb" into "test al,immediate"

;	 while (ch = *qch++)
;		 *pch++ = ch;
PSRM01:
	stosb
	lodsb
	or	al,al
	jne	PSRM01

;	 return pch;
	push	ss
	pop	ds
	xchg	ax,di

;}
	pop	di	    ;restore caller's di
	pop	si	    ;restore caller's si
ifdef PROFILE
	call	StopNMeas
	pop	ds
	pop	bp
	dec	bp
endif ;PROFILE
	db	0CAh, 004h, 000h    ;far ret, pop 4 bytes

; End of PchSzRtfMove


;-------------------------------------------------------------------------
;	FSearchRgrsym(sz, pirsym)
;-------------------------------------------------------------------------
;/* F  S E A R C H  R G R S Y M
;*  Binary-search string table returning true if the string is found.
;*  In any case return the index of where the string would be in rgbst at *pirsym.
;   Note the assumption that rgrsym and rgszRtfSym are in 1 to 1
;   correspondence so searching rgszRtfSym is equivalent to searching rgsym.
;*/
;
;HANDNATIVE int C_FSearchRgrsym(sz, pirsym)
;char *sz;
;int *pirsym;
;{
;   int irsymMin = 0;
;   int irsym = irsymMin;
;   int irsymLim = iszRTFMax;
;   int wCompGuess;
;   CHAR FAR *qch;
;   CHAR *pch;
; %%Function:N_FSearchRgrsym %%Owner:BRADV
PUBLIC N_FSearchRgrsym
N_FSearchRgrsym:
	mov	bx,sp
	mov	cx,[bx+4]
	mov	bx,[bx+6]
ifdef PROFILE
	inc	bp
	push	bp
	mov	bp,sp
	push	ds
	call	StartNMeas
endif ;PROFILE
	push	si	;save caller's si
	push	di	;save caller's di

	push	cx	;save pirsym
	push	ds
	pop	es
	mov	di,bx
	mov	si,di
	xor	ax,ax	    ;irsymMin
	mov	cx,-1
	repne	scasb
	not	cx
	mov	dx,iszRTFMax SHL 1	;irsymLim
	push	cs
	pop	es
	db	03Dh	;turns next "mov dx,bx" into "cmp ax,immediate"

;   while (irsymMin < irsymLim)
;	{
FSR03:
	mov	dx,bx
FSR04:
	cmp	ax,dx
	jae	FSR06

;	irsym = (irsymMin + irsymLim) >> 1;
	mov	bx,ax
	add	bx,dx
	shr	bx,1
	and	bl,0FEh

;	/* open code WCompSzQsz */
;	qch = rgszRtfSym[irsym];
	mov	di,[bx.rgszRtfSym]

;	pch = sz;
;	while (*pch == *qch)
;	    {
;	    if (*pch == 0)
;		{
;		*pirsym = irsym;
;		return fTrue;	    /* found: return index */
;		}
;	    pch++;
;	    qch++;
;	    }
;
;	if ((wCompGuess = (*pch - *qch)) < 0)
;	    irsymLim = irsym;
;	else
;	    irsymMin = ++irsym;
;	}
	push	cx
	push	si
	repe	cmpsb
	pop	si
	pop	cx
	je	FSR05
	jb	FSR03
	inc	bx
	inc	bx
	mov	ax,bx
	jmp	short FSR04

FSR05:
	db	0B8h	;turns next "xor ax,ax" into "mov ax,immediate"
FSR06:

;   *pirsym = irsym;
;   return fFalse;		    /* not found: return insert point */
;}
	errnz	<fFalse>
	xor	ax,ax
	pop	si	;restore pirsym
	shr	bx,1
	mov	[si],bx

	pop	di	    ;restore caller's di
	pop	si	    ;restore caller's si
ifdef PROFILE
	call	StopNMeas
	pop	ds
	pop	bp
	dec	bp
endif ;PROFILE
	db	0CAh, 004h, 000h    ;far ret, pop 4 bytes

; End of FSearchRgrsym

sEnd	rtfsubsn
        end
