
state 0
	$accept : _fexp $end 

	EQ  shift 3
	FLDESC  shift 2
	.  error

	fexp  goto 1

state 1
	$accept :  fexp_$end 

	$end  accept
	.  error


state 2
	fexp :  FLDESC_EQ exp 

	EQ  shift 4
	.  error


state 3
	fexp :  EQ_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 5
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 4
	fexp :  FLDESC EQ_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 21
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 5
	fexp :  EQ exp_    (2)
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 2


state 6
	exp :  -_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 33
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 7
	exp :  BOOL_    (15)

	.  reduce 15


state 8
	exp :  ucnum_    (16)
	ucnum :  ucnum_ucn 

	NUMBER  shift 18
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	.  reduce 16

	cur_num  goto 16
	ucn  goto 34

state 9
	exp :  BOOKMARK_    (17)

	.  reduce 17


state 10
	exp :  (_exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 35
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 11
	exp :  RDCFNC_( tbl_ref ) 

	(  shift 36
	.  error


state 12
	exp :  BINFNC_( exp COMSEP exp ) 

	(  shift 37
	.  error


state 13
	exp :  UNRFNC_$$21 ( $$22 exp ) 
	$$21 : _    (21)

	.  reduce 21

	$$21  goto 38

state 14
	exp :  IF_( exp COMSEP exp COMSEP exp ) 

	(  shift 39
	.  error


state 15
	ucnum :  ucn_    (58)

	.  reduce 58


state 16
	ucn :  cur_num_    (53)

	.  reduce 53


state 17
	ucn :  NUMINPAREN_    (54)

	.  reduce 54


state 18
	cur_num :  NUMBER_POST_CURRENCY 
	ucn :  NUMBER_    (55)

	POST_CURRENCY  shift 40
	.  reduce 55


state 19
	cur_num :  INTEGER_POST_CURRENCY 
	ucn :  INTEGER_    (56)

	POST_CURRENCY  shift 41
	.  reduce 56


state 20
	cur_num :  PRE_CURRENCY_NUMBER 
	cur_num :  PRE_CURRENCY_INTEGER 

	NUMBER  shift 42
	INTEGER  shift 43
	.  error


state 21
	fexp :  FLDESC EQ exp_    (1)
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 1


state 22
	exp :  exp EQ_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 44
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 23
	exp :  exp LT_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 45
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 24
	exp :  exp LE_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 46
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 25
	exp :  exp GT_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 47
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 26
	exp :  exp GE_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 48
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 27
	exp :  exp NE_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 49
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 28
	exp :  exp +_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 50
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 29
	exp :  exp -_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 51
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 30
	exp :  exp *_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 52
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 31
	exp :  exp /_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 53
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 32
	exp :  exp ^_exp 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 54
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 33
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  - exp_    (14)

	.  reduce 14


state 34
	ucnum :  ucnum ucn_    (57)

	.  reduce 57


state 35
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  ( exp_) 

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	)  shift 55
	.  error


state 36
	exp :  RDCFNC (_tbl_ref ) 

	NUMBER  shift 18
	BOOKMARK  shift 58
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	[  shift 62
	.  error

	exp  goto 61
	ucnum  goto 8
	tbl_ref  goto 56
	bkmk_rel  goto 57
	l_bracket  goto 60
	texp  goto 59
	cur_num  goto 16
	ucn  goto 15

state 37
	exp :  BINFNC (_exp COMSEP exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 63
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 38
	exp :  UNRFNC $$21_( $$22 exp ) 

	(  shift 64
	.  error


state 39
	exp :  IF (_exp COMSEP exp COMSEP exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 65
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 40
	cur_num :  NUMBER POST_CURRENCY_    (50)

	.  reduce 50


state 41
	cur_num :  INTEGER POST_CURRENCY_    (52)

	.  reduce 52


state 42
	cur_num :  PRE_CURRENCY NUMBER_    (49)

	.  reduce 49


state 43
	cur_num :  PRE_CURRENCY INTEGER_    (51)

	.  reduce 51


state 44
	exp :  exp_EQ exp 
	exp :  exp EQ exp_    (3)
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 3


state 45
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp LT exp_    (4)
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 4


state 46
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp LE exp_    (5)
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 5


state 47
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp GT exp_    (6)
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 6


state 48
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp GE exp_    (7)
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 7


state 49
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp NE exp_    (8)
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 8


state 50
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp + exp_    (9)
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 9


state 51
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp - exp_    (10)
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 10


state 52
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp * exp_    (11)
	exp :  exp_/ exp 
	exp :  exp_^ exp 

	^  shift 32
	.  reduce 11


state 53
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp / exp_    (12)
	exp :  exp_^ exp 

	^  shift 32
	.  reduce 12


state 54
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  exp ^ exp_    (13)

	^  shift 32
	.  reduce 13


state 55
	exp :  ( exp )_    (18)

	.  reduce 18


state 56
	exp :  RDCFNC ( tbl_ref_) 
	texp :  tbl_ref_    (47)

	)  shift 66
	.  reduce 47


state 57
	tbl_ref :  bkmk_rel_    (25)

	.  reduce 25


state 58
	exp :  BOOKMARK_    (17)
	tbl_ref :  BOOKMARK_l_bracket r_bracket 
	bkmk_rel :  BOOKMARK_l_bracket loc r_bracket 
	bkmk_rel :  BOOKMARK_l_bracket locbk : locbk r_bracket 

	[  shift 62
	.  reduce 17

	l_bracket  goto 67

state 59
	tbl_ref :  texp_COMSEP texp 

	COMSEP  shift 68
	.  error


state 60
	bkmk_rel :  l_bracket_loc r_bracket 
	bkmk_rel :  l_bracket_locbk : locbk r_bracket 

	BOOKMARK  shift 73
	ROW  shift 71
	COL  shift 72
	.  error

	loc  goto 69
	locbk  goto 70

state 61
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	texp :  exp_    (48)

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  reduce 48


state 62
	l_bracket :  [_    (28)

	.  reduce 28


state 63
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  BINFNC ( exp_COMSEP exp ) 

	COMSEP  shift 74
	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  error


state 64
	exp :  UNRFNC $$21 (_$$22 exp ) 
	$$22 : _    (22)

	.  reduce 22

	$$22  goto 75

state 65
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  IF ( exp_COMSEP exp COMSEP exp ) 

	COMSEP  shift 76
	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  error


state 66
	exp :  RDCFNC ( tbl_ref )_    (19)

	.  reduce 19


state 67
	tbl_ref :  BOOKMARK l_bracket_r_bracket 
	bkmk_rel :  BOOKMARK l_bracket_loc r_bracket 
	bkmk_rel :  BOOKMARK l_bracket_locbk : locbk r_bracket 

	BOOKMARK  shift 73
	ROW  shift 71
	COL  shift 72
	]  shift 80
	.  error

	r_bracket  goto 77
	loc  goto 78
	locbk  goto 79

state 68
	tbl_ref :  texp COMSEP_texp 

	NUMBER  shift 18
	BOOKMARK  shift 58
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	[  shift 62
	.  error

	exp  goto 61
	ucnum  goto 8
	tbl_ref  goto 82
	bkmk_rel  goto 57
	l_bracket  goto 60
	texp  goto 81
	cur_num  goto 16
	ucn  goto 15

state 69
	bkmk_rel :  l_bracket loc_r_bracket 
	locbk :  loc_    (42)

	]  shift 80
	.  reduce 42

	r_bracket  goto 83

state 70
	bkmk_rel :  l_bracket locbk_: locbk r_bracket 

	:  shift 84
	.  error


state 71
	loc :  ROW_sint COL sint 
	loc :  ROW_sint 
	loc :  ROW_COL sint 
	loc :  ROW_sint COL 
	loc :  ROW_COL 
	loc :  ROW_    (40)

	COL  shift 86
	INTEGER  shift 87
	+  shift 89
	-  shift 88
	.  reduce 40

	sint  goto 85

state 72
	loc :  COL_sint 
	loc :  COL_    (41)

	INTEGER  shift 87
	+  shift 89
	-  shift 88
	.  reduce 41

	sint  goto 90

state 73
	locbk :  BOOKMARK_    (43)

	.  reduce 43


state 74
	exp :  BINFNC ( exp COMSEP_exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 91
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 75
	exp :  UNRFNC $$21 ( $$22_exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 92
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 76
	exp :  IF ( exp COMSEP_exp COMSEP exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 93
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 77
	tbl_ref :  BOOKMARK l_bracket r_bracket_    (26)

	.  reduce 26


state 78
	bkmk_rel :  BOOKMARK l_bracket loc_r_bracket 
	locbk :  loc_    (42)

	]  shift 80
	.  reduce 42

	r_bracket  goto 94

state 79
	bkmk_rel :  BOOKMARK l_bracket locbk_: locbk r_bracket 

	:  shift 95
	.  error


state 80
	r_bracket :  ]_    (29)

	.  reduce 29


state 81
	tbl_ref :  texp_COMSEP texp 
	tbl_ref :  texp COMSEP texp_    (27)

	.  reduce 27


state 82
	texp :  tbl_ref_    (47)

	.  reduce 47


state 83
	bkmk_rel :  l_bracket loc r_bracket_    (32)

	.  reduce 32


state 84
	bkmk_rel :  l_bracket locbk :_locbk r_bracket 

	BOOKMARK  shift 73
	ROW  shift 71
	COL  shift 72
	.  error

	loc  goto 97
	locbk  goto 96

state 85
	loc :  ROW sint_COL sint 
	loc :  ROW sint_    (35)
	loc :  ROW sint_COL 

	COL  shift 98
	.  reduce 35


state 86
	loc :  ROW COL_sint 
	loc :  ROW COL_    (39)

	INTEGER  shift 87
	+  shift 89
	-  shift 88
	.  reduce 39

	sint  goto 99

state 87
	sint :  INTEGER_    (44)

	.  reduce 44


state 88
	sint :  -_INTEGER 

	INTEGER  shift 100
	.  error


state 89
	sint :  +_INTEGER 

	INTEGER  shift 101
	.  error


state 90
	loc :  COL sint_    (36)

	.  reduce 36


state 91
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  BINFNC ( exp COMSEP exp_) 

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	)  shift 102
	.  error


state 92
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  UNRFNC $$21 ( $$22 exp_) 

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	)  shift 103
	.  error


state 93
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  IF ( exp COMSEP exp_COMSEP exp ) 

	COMSEP  shift 104
	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	.  error


state 94
	bkmk_rel :  BOOKMARK l_bracket loc r_bracket_    (30)

	.  reduce 30


state 95
	bkmk_rel :  BOOKMARK l_bracket locbk :_locbk r_bracket 

	BOOKMARK  shift 73
	ROW  shift 71
	COL  shift 72
	.  error

	loc  goto 97
	locbk  goto 105

state 96
	bkmk_rel :  l_bracket locbk : locbk_r_bracket 

	]  shift 80
	.  error

	r_bracket  goto 106

state 97
	locbk :  loc_    (42)

	.  reduce 42


state 98
	loc :  ROW sint COL_sint 
	loc :  ROW sint COL_    (38)

	INTEGER  shift 87
	+  shift 89
	-  shift 88
	.  reduce 38

	sint  goto 107

state 99
	loc :  ROW COL sint_    (37)

	.  reduce 37


state 100
	sint :  - INTEGER_    (45)

	.  reduce 45


state 101
	sint :  + INTEGER_    (46)

	.  reduce 46


state 102
	exp :  BINFNC ( exp COMSEP exp )_    (20)

	.  reduce 20


state 103
	exp :  UNRFNC $$21 ( $$22 exp )_    (23)

	.  reduce 23


state 104
	exp :  IF ( exp COMSEP exp COMSEP_exp ) 

	NUMBER  shift 18
	BOOKMARK  shift 9
	RDCFNC  shift 11
	BINFNC  shift 12
	UNRFNC  shift 13
	IF  shift 14
	INTEGER  shift 19
	PRE_CURRENCY  shift 20
	NUMINPAREN  shift 17
	BOOL  shift 7
	-  shift 6
	(  shift 10
	.  error

	exp  goto 108
	ucnum  goto 8
	cur_num  goto 16
	ucn  goto 15

state 105
	bkmk_rel :  BOOKMARK l_bracket locbk : locbk_r_bracket 

	]  shift 80
	.  error

	r_bracket  goto 109

state 106
	bkmk_rel :  l_bracket locbk : locbk r_bracket_    (33)

	.  reduce 33


state 107
	loc :  ROW sint COL sint_    (34)

	.  reduce 34


state 108
	exp :  exp_EQ exp 
	exp :  exp_LT exp 
	exp :  exp_LE exp 
	exp :  exp_GT exp 
	exp :  exp_GE exp 
	exp :  exp_NE exp 
	exp :  exp_+ exp 
	exp :  exp_- exp 
	exp :  exp_* exp 
	exp :  exp_/ exp 
	exp :  exp_^ exp 
	exp :  IF ( exp COMSEP exp COMSEP exp_) 

	EQ  shift 22
	LT  shift 23
	LE  shift 24
	GT  shift 25
	GE  shift 26
	NE  shift 27
	+  shift 28
	-  shift 29
	*  shift 30
	/  shift 31
	^  shift 32
	)  shift 110
	.  error


state 109
	bkmk_rel :  BOOKMARK l_bracket locbk : locbk r_bracket_    (31)

	.  reduce 31


state 110
	exp :  IF ( exp COMSEP exp COMSEP exp )_    (24)

	.  reduce 24


34/127 terminals, 15/100 nonterminals
59/200 grammar rules, 111/450 states
0 shift/reduce, 0 reduce/reduce conflicts reported
41/125 working sets used
memory: states,etc. 1025/1500, parser 129/1000
31/200 distinct lookahead sets
49 extra closures
487 shift entries, 1 exceptions
52 goto entries
71 entries saved by goto default
Optimizer space used: input 1065/1500, output 311/1000
311 table entries, 92 zero
maximum spread: 277, maximum offset: 275
