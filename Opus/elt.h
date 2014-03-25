#define eltNil		0

#define eltMinus	1
#define eltNot		2

/* #define eltUnaryLim	3 */

#define eltAnd		3
#define eltOr		4
#define eltLParen	5
#define eltRParen	6
#define eltAdd		7
#define eltSubtract	8
#define eltDiv		9
#define eltMul		10
#define eltMod		11
#define eltEq		12
#define eltNe		13
#define eltLt		14
#define eltGt		15
#define eltLe		16
#define eltGe		17
#define eltComma	18
#define eltSubscript	19	/* call or array reference */
#define eltOperand	20	/* push an operand */
#define eltElkGets	21	/* for internal EXP use */
#define eltEndExp	22	/* end of expression */

/* #define eltOprLim	23 */

#define eltEol		23
#define eltResume	24
#define eltColon	25
#define eltEnd		26
#define eltSub		27
#define eltFunction	28
#define eltIf		29
#define eltThen		30
#define eltElseif	31
#define eltElse		32
#define eltWhile	33
#define eltWend		34
#define eltFor		35
#define eltTo		36
#define eltStep		37
#define eltNext		38
#define eltExit		39
#define eltSemi		40
#define eltCall		41
#define eltGoto		42
#define eltStop		43
#define eltOn		44
#define eltError	45
#define eltLet		46
#define eltDim		47
#define eltShared	48
#define eltSelect	49
#define eltIs		50	/* for CASE statements */
#define eltCase		51
#define eltAs		52	/* for DIM ... AS ... */
#define eltRedim	53
#define eltPrint	54
#define eltInput	55
#define eltLine		56
#define eltWrite	57
#define eltName		58
#define eltOutput	59
#define eltAppend	60
#define eltOpen		61
#define eltGetCurVals	62
#define eltDialog	63
#define eltSuper	64
#define eltDeclare	65
#define eltDouble	66
#define eltInteger	67
#define eltLong		68
#define eltSingle	69
#define eltStringT	70
#define eltCdecl	71
#define eltAlias	72
#define eltAny		73
#define eltSpell	74
#define eltThes		75
#define eltListBox	76
#define eltComboBox	77
#define eltClose	78
#define eltBegin	79
#define eltLib		80
#define eltTextBox	81
#define eltOptGroup	82
#define eltCheckBox	83
#define eltRead		84

/* #define eltElccMin	85 */

#define eltLineNolabel	85
#define eltLineIdent	86
#define eltLineInt	87
#define eltElt		88
#define eltElx		89
#define eltElk		90
#define eltNum		91
#define eltIdent	92
#define eltString	93
#define eltComment	94
#define eltInt		95
#define eltEof		96
#define eltStmtSep	97
#define eltBadSyntax	98
#define eltSpace	99
#define eltRem		100
#define eltHash		101
#define eltUserElk	102
